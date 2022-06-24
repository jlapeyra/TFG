import matplotlib.pyplot as plt
import pandas as pd
import sys

#USAGE: python3 grafic_resultats.py lin|log [CP13]

if len(sys.argv) > 2:
    num_inst = 16
else:
    num_inst = 109

df = pd.read_excel('wcsp-maxsat.ods')
for label in df.columns:
    list = []
    for x in df[label][:num_inst]:
        if type(x) in [int, float]:
            list.append(x)
    list.sort()
    plt.plot(range(1,len(list)+1), list, label=label)

plt.legend()
add = ''
if len(sys.argv) > 2: add=' en instàncies CP13'
plt.title('Eficiència de solvers MaxSAT i WCSP'+add)
plt.ylabel('temps (segons)')
plt.xlabel('nombre d\'instàncies resoltes')
if sys.argv[1] == 'log':
    plt.yscale('log')
plt.show()