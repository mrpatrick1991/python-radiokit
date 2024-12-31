from radiokit.models import itm
from radiokit.terrain import opentopography as ot
import random
from timeit import default_timer as timer

from dotenv import dotenv_values
config = dotenv_values(".env")


# Example parameters
h_tx = 20.0
h_rx = 10.0
distance_km = 100.0  # Path length in kilometers
pfl = [random.uniform(0, 10) for _ in range(100)]  # Random terrain profile
climate = "continental_temperate"
N_0 = 301.0
f_mhz = 900.0
pol = 1
epsilon = 15.0
sigma = 0.005
mdvar = 1
time = 0.5
location = 0.5
situation = 0.5

print(pfl)
start = timer()

result = itm.point_to_point(
    h_tx, h_rx, pfl, distance_km, climate, N_0, f_mhz, pol, epsilon, sigma, mdvar, time, location, situation
)

end = timer()
print(result)
print(end-start)


data = ot.fetch(dataset="SRTMGL3", lat=51.0447 ,lon=-114.0719,radius_km=1, api_key=config["OPENTOPO_API_KEY"])

with open("foo.tif","wb") as f:
    f.write(data)

