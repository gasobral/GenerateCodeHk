# Density of identifying codes of hexagonal grids with finite number of rows
Shield: [![CC BY 4.0][cc-by-shield]][cc-by]

This work is licensed under a
[Creative Commons Attribution 4.0 International License][cc-by].

[![CC BY 4.0][cc-by-image]][cc-by]

[cc-by]: http://creativecommons.org/licenses/by/4.0/
[cc-by-image]: https://i.creativecommons.org/l/by/4.0/88x31.png
[cc-by-shield]: https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg


## Description
During the research about identifying codes (*idcode* for short) in
hexagonal grids with fine number of rows, we developed a few programs to
find such *idcodes*. The codes
[Hk_lemon_eng_8bar.cc](GenerateCodeHk/GenerateCodeHk/Hk_lemon_eng_8bar.cc)
and
[Hk_lemon_eng_6bar.cc](GenerateCodeHk/GenerateCodeHk/Hk_lemon_eng_6bar.cc)
use the concepts *configuration graph* and *bar code* to find *idcodes*
with minimum densities for $H_k$, the hexagonal grid with $k$ rows
(where $k \geq 2$). *Discharging Method* is applied to ensure those
codes have the minimum density. Whereas the code
[GenerateHkCode.py](GenerateCodeHk/GenerateCodeHk/GenerateHkCode.py)
uses a linear program to find *idcodes* for $H_k$. They are not
necessarily *idcodes* with minimum density, but they can provide upper
bounds for them. Below you can find a table with a list of the programs
and their description and how to use them. For more information about
the research, please check this
[paper](https://www.rairo-ro.org/articles/ro/abs/2024/02/ro230161/ro230161.html)
published in RAIO and this
[thesis](https://www.teses.usp.br/teses/disponiveis/45/45134/tde-18092024-200458/pt-br.php)
(only in brazilian portuguese).


| Implementation | Description |
| ----------- | ----------- |
| [Hk_lemon_eng_8bar.cc](GenerateCodeHk/GenerateCodeHk/Hk_lemon_eng_8bar.cc) | Finds an *idcode* with minimum density using a configuration graph using barcodes of length 8 |
| [Hk_lemon_eng_6bar.cc](GenerateCodeHk/GenerateCodeHk/Hk_lemon_eng_6bar.cc) | Finds an *idcode* with minimum density using a configuration graph using barcodes of length 6 |
| [GenerateHkCode.py](GenerateCodeHk/GenerateCodeHk/GenerateHkCode.py) | Finds an *idcode* using a linear program (guroby) |


## How to Use
For the codes
[Hk_lemon_eng_8bar.cc](GenerateCodeHk/GenerateCodeHk/Hk_lemon_eng_8bar.cc)
and
[Hk_lemon_eng_6bar.cc](GenerateCodeHk/GenerateCodeHk/Hk_lemon_eng_6bar.cc)
is required to install [lemon
libray](https://lemon.cs.elte.hu/trac/lemon). Once they are compiled,
just provide the number of lines of the hexagonal grid as an argument in
the command line. When program terminates its execution, it will output
the vertices which belongs to periodic identifying code. The pattern of
the *idcode* is saved at [Codes](Codes) directory. The script
[GenerateHkCode.py](GenerateCodeHk/GenerateCodeHk/GenerateHkCode.py)
requires *networkx* and *guroby*. In order to execute it, just provide
the number of rows and columns. The will output the patter of the
*idcode* once it terminates. Below you can find project structure.

```bash
(Project Root)
├── Codes                       ## a directory with codes found by the program
│   ├── CodigoH2GrafoConfig.txt
│   ├── CodigoH2PL.txt
│   ├── CodigoH3GrafoConfig.txt
│   ├── CodigoH3PL.txt
│   ├── CodigoH4GrafoConfig.txt
│   ├── CodigoH4PL.txt
│   ├── CodigoH5PL.txt
│   ├── CodigoH6PL.txt
│   └── CodigoH7PL.txt
├── GenerateCodeHk              ## programs which generate *idcodes*
│   ├── GenerateHkCode.py
│   ├── gurobi.env
│   ├── Hk_lemon_eng_6bar.cc
│   ├── Hk_lemon_eng_8bar.cc
│   ├── __init__.py
│   ├── RegularGrids            ## implementation of hexagonal grids using networkx
│   │   ├── HexagonalGrid.py
│   │   ├── __init__.py
│   │   └── __pycache__
│   │       ├── HexagonalGrid.cpython-313.pyc
│   │       └── __init__.cpython-313.pyc
│   └── Utilities               ## script which shows an *idcode* using matplotlib
│       ├── DrawCode.py
│       ├── __init__.py
│       └── __pycache__
│           ├── DrawCode.cpython-313.pyc
│           └── __init__.cpython-313.pyc
├── LICENSE
├── pyproject.toml
└── README.md

7 directories, 25 files
```
