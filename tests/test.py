import radiokit.bindings

# Define input parameters
h_tx = 80.0  # Transmitter height in meters
h_rx = 10.0  # Receiver height in meters
pfl = [19, 200, 50, 45, 48, 52, 54, 53, 51, 47, 46, 49, 50, 48, 47, 46, 44, 43, 45, 47]
climate = 5  # Climate category (e.g., Continental Temperate)
N_0 = 301.0  # Surface refractivity (N-units)
f_mhz = 900.0  # Frequency in MHz
pol = 0  # Polarization: 0 for horizontal, 1 for vertical
epsilon = 15.0  # Relative permittivity
sigma = 0.005  # Conductivity (S/m)
mdvar = 0  # Mode of variability: 0 for broadcasting
time = 50.0  # Time percentage
location = 50.0  # Location percentage
situation = 50.0  # Situation percentage

# Call itm_p2p_tls
status, A_db, warnings = radiokit.bindings.itm_p2p_tls(
    h_tx, h_rx, pfl, climate, N_0, f_mhz, pol, epsilon, sigma, mdvar, time, location, situation
)

# Display results
print("Status Code:", status)
print("Attenuation (dB):", A_db)
print("Warnings:")
for warning in warnings:
    print(" -", warning)
