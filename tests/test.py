from radiokit.models import itm
from radiokit.terrain import opentopography as ot
import random
from timeit import default_timer as timer
from dotenv import dotenv_values
import h3

config = dotenv_values(".env")

# Example parameters
h_tx = 5.0
h_rx = 1.0
climate = "continental_temperate"
N_0 = 301.0
f_mhz = 907.0
pol = 1
epsilon = 15.0
sigma = 0.005
mdvar = 1
time = 0.90
location = 0.90
situation = 0.90

home_geo = (40.0447,-110.0719)
home = h3.latlng_to_cell(home_geo[0], home_geo[1], res=12)

gt = ot.fetch(dataset="SRTMGL3", lat=home_geo[0], lon=home_geo[1], radius_m=1000*100, resolution=12, api_key=config["OPENTOPO_API_KEY"])

print("search area has cells: ", len(gt))

print("start area mode test")

start = timer()
results = {}
for cell, elevation in gt.items():

    if cell == home:
        continue


    path = h3.grid_path_cells(home, cell)
    distance = h3.great_circle_distance(home_geo, h3.cell_to_latlng(cell), unit='km')

    if distance >= 100.0:
        continue

    print("path: ", len(path))
    print("distance: ", distance)

    pfl = []
    for h in path:
        try:
            pfl.append(gt[h])
        except KeyError:
            pfl.append(0)

    print(pfl)

    result = itm.point_to_point(
        h_tx, h_rx, pfl, distance, climate, N_0, f_mhz, pol, epsilon, sigma, mdvar, time, location, situation
    )

    results[cell] = result["loss_db"]
    break
end = timer()

print("eval time: ", end-start)

with open("loss.csv","w") as f:
    for item in results.items():
        f.write("{},{}\n".format(item[0], item[1]))