from typing import List, Literal
from pydantic import BaseModel, Field, model_validator
from radiokit.bindings import itm_bindings


class PointToPointModel(BaseModel):
    h_tx: float = Field(..., description="Transmitter height in meters")
    h_rx: float = Field(..., description="Receiver height in meters")
    pfl: List[float] = Field(
        ..., description="Terrain profile, a list of elevation samples in meters"
    )
    distance_km: float = Field(..., description="Path distance in kilometers")
    climate: Literal[
        "equatorial",
        "continental_subtropical",
        "maritime_tropical",
        "desert",
        "continental_temperate",
        "maritime_temperate_over_land",
        "maritime_temperate_over_sea",
    ] = Field(..., description="Climate zone for the path")
    N_0: float = Field(..., description="Surface refractivity in N-units")
    f_mhz: float = Field(..., description="Frequency in MHz")
    pol: Literal[0, 1] = Field(
        ..., description="Polarization: 0 for horizontal, 1 for vertical"
    )
    epsilon: float = Field(..., description="Relative permittivity")
    sigma: float = Field(..., description="Conductivity in S/m")
    mdvar: int = Field(..., description="Mode variability parameter")
    time: float = Field(
        ..., description="Percentage of time for which predictions are desired (0-1)"
    )
    location: float = Field(
        ...,
        description="Percentage of locations for which predictions are desired (0-1)",
    )
    situation: float = Field(
        ...,
        description="Percentage of situations for which predictions are desired (0-1)",
    )

    @model_validator(mode="before")
    def validate_pfl_and_spacing(cls, values):
        pfl = values.get("pfl")
        distance_km = values.get("distance_km")

        if not pfl or not distance_km:
            raise ValueError("Both 'pfl' and 'distance_km' must be provided.")

        spacing = distance_km * 1000 / (len(pfl) - 1)
        pfl_with_spacing = [len(pfl), spacing, *pfl]
        values["pfl"] = pfl_with_spacing
        return values


# Mapping climate strings to integer codes
CLIMATE_MAPPING = {
    "equatorial": 1,
    "continental_subtropical": 2,
    "maritime_tropical": 3,
    "desert": 4,
    "continental_temperate": 5,
    "maritime_temperate_over_land": 6,
    "maritime_temperate_over_sea": 7,
}


def point_to_point(
    h_tx: float,
    h_rx: float,
    pfl: List[float],
    distance_km: float,
    climate: str,
    N_0: float,
    f_mhz: float,
    pol: int,
    epsilon: float,
    sigma: float,
    mdvar: int,
    time: float,
    location: float,
    situation: float,
) -> dict:
    if climate not in CLIMATE_MAPPING:
        raise ValueError(
            f"Invalid climate: {climate}. Must be one of {list(CLIMATE_MAPPING.keys())}"
        )

    climate_code = CLIMATE_MAPPING[climate]

    inputs = PointToPointModel(
        h_tx=h_tx,
        h_rx=h_rx,
        pfl=pfl,
        distance_km=distance_km,
        climate=climate,
        N_0=N_0,
        f_mhz=f_mhz,
        pol=pol,
        epsilon=epsilon,
        sigma=sigma,
        mdvar=mdvar,
        time=time,
        location=location,
        situation=situation,
    )

    # Call the C++ binding
    status, loss_db, warnings = itm_bindings.itm_p2p_tls(
        inputs.h_tx,
        inputs.h_rx,
        inputs.pfl,
        climate_code,  # Pass the integer climate code
        inputs.N_0,
        inputs.f_mhz,
        inputs.pol,
        inputs.epsilon,
        inputs.sigma,
        inputs.mdvar,
        inputs.time,
        inputs.location,
        inputs.situation,
    )

    return {
        "status": status,
        "loss_db": loss_db,
        "warnings": warnings,
    }
