# Identifying Codes
Shield: [![CC BY 4.0][cc-by-shield]][cc-by]

This work is licensed under a
[Creative Commons Attribution 4.0 International License][cc-by].

[![CC BY 4.0][cc-by-image]][cc-by]

[cc-by]: http://creativecommons.org/licenses/by/4.0/
[cc-by-image]: https://i.creativecommons.org/l/by/4.0/88x31.png
[cc-by-shield]: https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg

## Descritpion
In this project we implemented a program which computes a configuration graph G
and finds a minimum mean cycle in G. This cycle is used to generate a periodic
identifying code, with minimum density, for the hexagonal grid with finite number
of rows. We two implementations:

| Implementation | Description |
| ----------- | ----------- |
| Hk_lemon_eng.cc | Creates a configuration graph using barcodes of length 8 |
| Hk_lemon_engV2.cc | Creates a configuration graph using barcodes of length 6 |

## Usage
There are two codes: Hk_lemon_eng.cc and Hk_lemon_engV2.cc. Once they are compiled,
which requirers [lemon libray](https://lemon.cs.elte.hu/trac/lemon), all you need to
pass is the number of lines of the hexagonal as an argument, in the command line.
The will output the vertices which belongs to periodic identifying code.


## Cloing the repositoy
```
cd existing_repo
git remote add origin https://gitlab.uspdigital.usp.br/gagsobral/identifying-codes.git
git branch -M main
git push -uf origin main
```
