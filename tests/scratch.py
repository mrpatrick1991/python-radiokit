
def fetch(
    dataset: str,
    lat: float,
    lon: float,
    radius_km: float,
    api_key: str,
    cache_dir: str = ".tilecache",
    cache_size_gb: int = 1,
) -> bytes:
    """
    Fetch digital elevation data from OpenTopography.

    Args:
        dataset (str): The DEM dataset name.
        lat (float): Latitude of the center point.
        lon (float): Longitude of the center point.
        radius_km (float): Radius around the center point in kilometers.
        api_key (str): API key for OpenTopography.
        cache_dir (str): Directory for the terrain tile / DEM cache.
        cache_size_gb (int): Maximum cache size in GB.

    Returns:
        GeoTIFF data as bytes.
    """
    # Initialize the disk cache
    cache = Cache(cache_dir, size_limit=cache_size_gb * 1024 * 1024 * 1024)

    # Calculate the bounding box
    bbox = _calculate_bbox(lat, lon, radius_km)

    # Validate the request
    request_data = OpenTopographyRequest(
        demtype=dataset,
        south=bbox[0],
        north=bbox[1],
        west=bbox[2],
        east=bbox[3],
        api_key=api_key,
    )

    cache_key = f"{dataset}_{bbox}"
    if cache_key in cache:
        return cache[cache_key]

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
    cache[cache_key] = geotiff_data

    return geotiff_data





from rasterio.warp import transform
from rasterio.io import MemoryFile


from pyproj import Geod
from typing import List, Tuple

import rasterio
import h3
from rasterio.transform import from_bounds
import numpy as np


start = (51.0447, -114.0719)
end = (51.06300798790394, -114.15311208731075)

spacing_km = .1  # Spacing in kilometers

points = generate_evenly_spaced_points(start, end, spacing_km)
print("Generated Points:")
for lat, lon in points:
    print(f"Lat: {lat}, Lon: {lon}")

with MemoryFile(gt) as memfile:
    with memfile.open() as src:
        lats, lons = zip(*points)

        # Batch transform all lat/lon to raster CRS
        xs, ys = transform("EPSG:4326", src.crs, lons, lats)

        # Convert all transformed coordinates to row/column indices
        rows, cols = src.indexes(xs, ys)

        # Read elevation values for all points
        values = []
        for row, col in zip(rows, cols):
            if 0 <= row < src.height and 0 <= col < src.width:
                values.append(src.read(1)[row, col])
            else:
                values.append(None)  # Add None for out-of-bounds points
        print(values)


def generate_evenly_spaced_points(
    start: Tuple[float, float],
    end: Tuple[float, float],
    spacing_km: float
) -> List[Tuple[float, float]]:
    """
    Generate a list of evenly spaced latitude/longitude points between two locations.

    Args:
        start (Tuple[float, float]): Starting point as (latitude, longitude).
        end (Tuple[float, float]): Ending point as (latitude, longitude).
        spacing_km (float): Distance in kilometers between each point.

    Returns:
        List[Tuple[float, float]]: List of (latitude, longitude) points.
    """
    geod = Geod(ellps="WGS84")

    # Calculate the forward azimuth, back azimuth, and total distance
    azimuth, _, total_distance = geod.inv(start[1], start[0], end[1], end[0])

    # Number of points needed (add 1 for the start point)
    num_points = int(total_distance // (spacing_km * 1000)) + 1

    # Generate intermediate points
    intermediate_points = geod.fwd_intermediate(
        lon1=start[1],
        lat1=start[0],
        azi1=azimuth,
        npts=num_points - 2,
        del_s=spacing_km * 1000,  # Convert km to meters
    )

    # Combine start, intermediate, and end points
    lons = [start[1]] + list(intermediate_points.lons) + [end[1]]
    lats = [start[0]] + list(intermediate_points.lats) + [end[0]]

    return list(zip(lats, lons))