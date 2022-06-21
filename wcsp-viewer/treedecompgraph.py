import networkx as nx
from networkx.drawing.nx_agraph import write_dot, graphviz_layout
import matplotlib.pyplot as plt
import os


def treedecompgraph(wcsp):
    treedecomp = '.tree-py.tmp'
    this_dir = os.path.dirname(os.path.abspath(__file__))
    os.system(f"{this_dir}/treedecomp.sh {wcsp} {treedecomp}")
    with open(treedecomp, "r") as f:
        G = nx.DiGraph()
        root = True
        lines = f.readlines()
        father = {}       # we use a dictionary bc we don't know the nb of clusters a priori (use list otherwise)
        for line in lines:
            lst = line.split()
            # #print(lst)
            if len(lst) > 0 and lst[0] == "cluster":   # in my .treedecomp format this is always true
                if root == True:
                    root = False
                    G.add_node(lst[1])

                # add node label : number of proper variables
                try:
                    i = lst.index('vars')
                    vars = lst[i + 1][1:-3]  #lst[i + 1].strip('}').strip('{'): it appears a ',' after striping '}' --> 'x08' ^H caracter {.., ^H}
                    if vars: 
                        vars = vars.split(',') 
                    #print(" vars ...: ", lst[i + 1], vars, len(vars))
                    G.nodes[lst[1]]['vars'] = '{0}({1})'.format(lst[1], len(vars)) #len(vars)
                except ValueError:
                    pass

                    
                # add separator label to father of the current node
                try:
                    i = lst.index('sep')
                    sep = lst[i + 1].replace('{', '').replace('}', '')
                    if sep:
                        sep = sep.split(",")
                    #print(" sep ...: ", lst[i + 1], sep, len(sep))
                    G[father[lst[1]]][lst[1]]['sep'] = len(sep)
                except ValueError:  # separator is not especified (only over root node)
                    pass


                # add son nodes (if any)
                try:
                    i = lst.index('sons')
                    sons = lst[i + 1].replace('{', '').replace('}', '')
                    if sons:
                        sons = sons.split(",") # lst[i + 1][1:-3].split(',')  #
                    #print(" sons ...: ", lst[i + 1], sons, len(sons))
                    for node in sons:
                        G.add_node(node)
                        G.add_edge(lst[1], node)
                        father[node] = lst[1]
                except ValueError:    # it doesnt have sons (it's a leaf)
                    pass
                #print('-----')
    
    os.system(f'rm -f {treedecomp}')

    return G