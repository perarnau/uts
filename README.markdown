Variations around the MPI Workstealing implementation of UTS
============================================================

This repo contains several versions of the MPI workstealing implementation of
the UTS benchmark.
These variations are all about two WS parameters:

- victim selection process
- number of chunks stolen

The original version: `mpi_workstealing.c` uses a round robin victim selection
process, and steals 1 chunk.

Versions with `half` in there names steal half the chunks.

Versions with `rand` uses the `rand()` function to select a victim.

Versions with `gslui` uses the GNU Scientific Library, uniform random
distribution to select a victim.

Versions with `gslrd` uses the GNU Scientific Library, and apply a weight one
the probability of a thief choosing someone, using the TOFU coordinates of a
process.

Versions with `fix` in the name correct a divide by zero bug when using
weights: two process on the same processor have the same coordinates.

There are trace and no trace versions of these programs. No trace contain `nt`
in the name.

`mpi_coords.c` is a simple MPI program printing the tofu coordinates of all
processes.

Use `diff` to check the small modifications made between the original
`mpi_workstealing.c` (renamed here `mpi_wsnt.c` for the no trace version) and
each file.
