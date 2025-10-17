# TFG

Aquest repositori conté el codi del Treball de Final de Grau de Joan Lapeyra, anomenat [Implementació d'algoritmes d'optimització per al disseny computacional de proteïnes](https://upcommons.upc.edu/handle/2117/373140), dirigit per Javier Larrosa, entregat el juny de 2022 a la Facultat d'Informàtica de Barcelona (FIB), Universitat Politècnica de Catalunya (UPC).



Contingut principal:

- `wcsp`: Solver per WCSP, implementat en C++
- `maxSAT`: Solver per MaxSAT, implementat en Python
- `wcsp-viewer`: Visualitzador d'instàncies WCSP

Contingut addicional:

- `instances_preproc`: Subconjunt de les instàncies que hem fet servir pels experiments de WCSP. En aquest directori ja estan preprocessades per *toulbar2*.
- `instances_wcnf`: Subconjunt de les instàncies que hem fet servir pels experiments de MaxSAT. Són les traduccions de les `instances_preproc` mitjançant `wcsp2maxsat`
- `wcsp2maxsat`: Scripts per traduïr de WCSP a MaxSAT.
