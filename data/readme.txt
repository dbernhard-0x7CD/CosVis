Here is some more information regarding units:
* Positions (x, y, z) are in units of Mpc/h
* Velocities (vx, vy, vz) are in units of km/s
* Mass is in internal code units 
* Internal energy (uu) is in units of (km/s)^2
* SPH smoothing length (hh) is in units of Mpc/h
* Molecular weight (mu) is dimensionless
* Density (rho) is in units of h^2*Msolar/Mpc^3 where Msolar is the mass of the Sun
* Gravitational potential (phi) is in internal code units
Note that the "h" in Mpc/h is equal to 0.71.

The relevant bits of the mask are as follows:
2nd bit: Denotes whether a particle is dark matter (0) or a baryon (1)
6th bit: Denotes if a baryon particle is also a star particle
7th bit: Denotes if a baryon particle is also a wind particle
8th bit: Denotes if a baryon particle is also a star-forming gas particle
9th bit: Denotes if a dark matter particle has been flagged as an active galactic nuclei (AGN)

Time steps are taken linearly in terms of the cosmological scale factor, a. This is related to the redshift, z, via a = 1/(1+z). The simulation starts at z = 200 and finishes at z = 0 after taking 625 steps. Hence, doing something like the following in python will tell you what the scale factor is for each of the 625 snapshots: a = numpy.linspace(1./201., 1., 626)[1:]

Equation for derived quantity:
temperature T = 4.8e5 * uu / (1+z)^3 [Kelvin]

a = 1/(1+z)
=> a * (1 + z) = 1
=> 1 + z = 1/a
=> z = 1/a - 1

