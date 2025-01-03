import requests
import math
from typing import Tuple, Literal
from pydantic import BaseModel, Field
from diskcache import Cache
from rasterio.io import MemoryFile
from typing import Dict
import h3

class OpenTopographyRequest(BaseModel):
    demtype: Literal[
        "SRTMGL3",
        "SRTMGL1",
        "SRTMGL1_E",
        "AW3D30",
        "AW3D30_E",
        "NASADEM",
        "COP30",
        "COP90",
        "EU_DTM",
        "GEDI_L3",
    ] = Field(..., description="OpenTopography digital elevation model (DEM)")
    south: float = Field(
        gt=-180, lt=180, description="WGS 84 bounding box south coordinates"
    )
    north: float = Field(
        gt=-180, lt=180, description="WGS 84 bounding box north coordinates"
    )
    west: float = Field(
        gt=-180, lt=180, description="WGS 84 bounding box west coordinates"
    )
    east: float = Field(
        gt=-180, lt=180, description="WGS 84 bounding box east coordinates"
    )
    api_key: str = Field(..., description="OpenTopography API Key")


def _calculate_bbox(
    lat: float, lon: float, radius_m: float
) -> Tuple[float, float, float, float]:
    """
    Calculate a lat/lon bounding box around a point with a given radius in meters.

    Args:
        lat (float): Latitude in degrees.
        lon (float): Longitude in degrees.
        radius_m (float): Radius in meters.

    Returns:
        Tuple[float, float, float, float]: Bounding box (south, north, west, east).
    """
    EARTH_RADIUS_KM = 6371.0  # Earth's radius in kilometers

    # Convert latitude and longitude to radians
    lat_rad = math.radians(lat)
    lon_rad = math.radians(lon)

    # Angular distance in radians for the given radius
    angular_distance = radius_m / (EARTH_RADIUS_KM*1000)

    # Calculate the latitude bounds
    lat_min_rad = lat_rad - angular_distance
    lat_max_rad = lat_rad + angular_distance

    # Calculate the longitude bounds (longitude varies with latitude)
    lon_min_rad = lon_rad - math.asin(math.sin(angular_distance) / math.cos(lat_rad))
    lon_max_rad = lon_rad + math.asin(math.sin(angular_distance) / math.cos(lat_rad))

    # Convert the results back to degrees
    south = math.degrees(lat_min_rad)
    north = math.degrees(lat_max_rad)
    west = math.degrees(lon_min_rad)
    east = math.degrees(lon_max_rad)

    return (south, north, west, east)
def fetch(
        dataset: str,
        lat: float,
        lon: float,
        radius_m: float,
        api_key: str,
        resolution: int = 8,
        cache_dir: str = ".tilecache",
        cache_size_gb: int = 1,
) -> Dict[str, int]:
    """
    Fetch digital elevation data from OpenTopography and return elevation values for H3 cells.

    Args:
        dataset (str): The DEM dataset name.
        lat (float): Latitude of the center point.
        lon (float): Longitude of the center point.
        radius_m (float): Radius around the center point in meters.
        api_key (str): API key for OpenTopography.
        resolution (int): H3 resolution level.
        cache_dir (str): Directory for the terrain tile / DEM cache.
        cache_size_gb (int): Maximum cache size in GB.

    Returns:
        Dict[str, float]: Dictionary with H3 cell IDs as keys and elevations as values.
    """
    # Initialize the disk cache
    cache = Cache(cache_dir, size_limit=cache_size_gb * 1024 * 1024 * 1024)

    # Calculate the bounding box
    bbox = _calculate_bbox(lat, lon, radius_m)

    # Validate the request
    request_data = OpenTopographyRequest(
        demtype=dataset,
        south=bbox[0],
        north=bbox[1],
        west=bbox[2],
        east=bbox[3],
        api_key=api_key,
    )

    cache_key = f"{dataset}_{bbox}_{resolution}"
    if cache_key in cache:
        return cache[cache_key]

    # Fetch GeoTIFF data
    url = "https://portal.opentopography.org/API/globaldem"
    params = {
        "demtype": request_data.demtype,
        "south": request_data.south,
        "north": request_data.north,
        "west": request_data.west,
        "east": request_data.east,
        "API_Key": request_data.api_key,
    }
    response = requests.get(url, params=params, stream=True)

    response.raise_for_status()
    geotiff_data = response.content

    # Process the GeoTIFF data
    h3_elevations = {}
    with MemoryFile(geotiff_data) as memfile:
        with memfile.open() as dataset:
            # Generate H3 cell IDs within the bounding box
            latitudes = [bbox[0] + i * (bbox[1] - bbox[0]) / 100 for i in range(101)]
            longitudes = [bbox[2] + i * (bbox[3] - bbox[2]) / 100 for i in range(101)]

            for lat in latitudes:
                for lon in longitudes:
                    h3_cell = h3.latlng_to_cell(lat, lon, resolution)
                    if h3_cell not in h3_elevations:  # Avoid duplicate queries
                        # Query elevation for the H3 cell center
                        row, col = dataset.index(lon, lat)
                        if 0 <= row < dataset.height and 0 <= col < dataset.width:
                            h3_elevations[h3_cell] = int(dataset.read(1)[row, col])

    # Cache and return the result
    cache[cache_key] = h3_elevations
    return h3_elevations

