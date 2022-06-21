import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
import warnings
import numpy as np
import os

from compute_info import compute_info
from treedecompgraph import treedecompgraph

def count_dict(key, dict:dict, add=1):
    if key in dict:
        dict[key] += add
    else:
        dict[key] = add

def sorted_dict(dict):
    return {k: dict[k] for k in sorted(dict)}

class InputException(Exception):
    pass


class WCSP_reader():

    def __init__(self, wcsp_file:str):
        ''' wcsp_file -- path to a .wcsp file '''
        self.wcsp_filename = wcsp_file
        self.wcsp_basename = os.path.basename(self.wcsp_filename)
        self.in_file = open(wcsp_file, "r")
        # Read main WCSP attributes
        self.name = self._read_word()
        self.N = self._read_int() #num vars
        self.max_domain_size = self._read_int()
        self.num_cost_func = self._read_int()
        self.UB = self._read_int() #initial global upper bound of the problem

        self.info = {}
        self.info['Number of variables'] = self.N
        self.info['Maximum domain size'] = self.max_domain_size
        self.info['Number of cost functions'] = self.num_cost_func 
        self.info['Top cost (global upper bound)'] = self.UB


    def compute_additional_info(self):
        ''' compute additional info (self.info : dict) '''
        self.info.update(compute_info(self.wcsp_filename))

    _in_list = []

    def _read_word(self):
        if len(self._in_list) == 0:
            line = '\n'
            while line.isspace():
                line = self.in_file.readline()
            if line == '':
                raise InputException()
            self._in_list = line.split()
        string = self._in_list[0]
        self._in_list = self._in_list[1:]
        return string
    
    def _read_int(self):
        return int(self._read_word())

    traversed = False

    def traverse_instance(self):
        ''' traverse WCSP instance and compute:
                - self.graph          : nx.Graph
                - self.undefaultness  : list
                - self.complexity     : list
                - self.domain_count         : dict
                - self.arity_cost_count     : dict    
                - self.arity_hardness_count : dict
                - self.arity_count          : dict
                - self.hardness_count       : dict
                - self.info['Maximum arity'] : int
        '''
        if self.traversed: return
        self.traversed = True

        G = nx.Graph()
        G.add_nodes_from(range(self.N))
        self.undefaultness = []
        self.complexity = []
        self.domain_count = {}         # {domain:count, ...}
        self.arity_count = {}          # {arity:count, ...} 
        self.arity_cost_count = {}     # {(arity,cost):count, ...}
        self.arity_hardness_count = {} # {(arity,hardness):count, ...} hardness:{'soft','mixt','hard'}
        self.hardness_count = {'hard':0, 'mixt':0, 'soft':0} # {hardness:count, ...} hardness:{'soft','mixt','hard'}

        try:
            domains = [self._read_int() for j in range(self.N)]
            for dom in domains:
                count_dict(dom, self.domain_count)

            for i_func in range(self.num_cost_func): 
                #Function
                arity = self._read_int()
                vars_f = [self._read_int() for j in range(arity)]
                marker = self._read_int()
                if marker != -1:
                    default_cost = marker
                    num_tuples_given = self._read_int()

                    for j in range(arity): #interaction graph
                        for k in range(j+1, arity):
                            G.add_edge(vars_f[j], vars_f[k])
                    num_tuples_total = 1 
                    for var in vars_f:
                        num_tuples_total *= domains[var] # potential num tuples
                    
                    if num_tuples_given < num_tuples_total:
                        cost_set = {default_cost}
                        count_dict((arity, default_cost), self.arity_cost_count, \
                            add = num_tuples_total - num_tuples_given)
                    else:
                        cost_set = set()
                    num_tuples_cost_not_default = 0

                    for j in range(num_tuples_given): 
                        #Tuple
                        for var in vars_f:
                            self._read_int()
                        tuple_cost = self._read_int()
                        cost_set.add(tuple_cost)
                        count_dict((arity, tuple_cost), self.arity_cost_count)
                        if tuple_cost != default_cost: 
                            num_tuples_cost_not_default += 1

                    if {self.UB,0} == cost_set: hardness = 'hard'
                    elif self.UB in cost_set:   hardness = 'mixt'
                    else:                       hardness = 'soft'
                    count_dict((arity, hardness), self.arity_hardness_count)
                    count_dict(arity, self.arity_count)
                    count_dict(hardness, self.hardness_count)

                    self.undefaultness.append(num_tuples_cost_not_default / num_tuples_total)
                    self.complexity.append(len(cost_set) / num_tuples_total)

                else:
                    keyword = self._read_word()
                    

        except InputException:
            if not 'i_func' in locals(): i_func = 0
            #print(f'WARNING: {self.num_cost_func} functions stated but {i_func} given.')
            raise Exception(f'Inconsistent wcsp input: ({self.num_cost_func} stated functions, {i_func} given)')


        self.info['Maximum arity'] = max(self.arity_count)
        self.arity_count = sorted_dict(self.arity_count)
        self.domain_count = sorted_dict(self.domain_count)
        self.graph = G
        self.in_file.close()
        
    din_a4 = (8.27, 11.69)

    def plot_tree(self):
        ''' plot tree decomposition'''
        title = 'tree - '+self.wcsp_basename

        G = treedecompgraph(self.wcsp_filename)
        fig = plt.figure('tree '+self.wcsp_filename, figsize = self.din_a4, dpi = 100, facecolor = 'w', edgecolor = 'k')
        plt.title(title)
        pos = graphviz_layout(G, prog = 'dot')
        nx.draw(G, pos, with_labels = False, arrows = True) #, node_size=600)
        edge_labels = nx.get_edge_attributes(G, 'sep') 
        #formatted_edge_labels = {(elem[0],elem[1]):edge_labels[elem] for elem in edge_labels} # use this to modify the tuple keyed dict if it has > 2 elements, else ignore
        nx.draw_networkx_edge_labels(G, pos, edge_labels, font_color = 'red') #=formatted_edge_labels,font_color='red')
        nx.draw_networkx_labels(G, pos, nx.get_node_attributes(G,'vars'), font_size=9, font_color='black')
        self.tree = G


    def plot_graph(self):
        ''' plot interaction graph '''   
        if not self.traversed:
            self.traverse_instance()
        title = 'graph - '+self.wcsp_basename

        G = self.graph
        fig = plt.figure('graph '+self.wcsp_filename, figsize = self.din_a4, dpi = 100, facecolor = 'w', edgecolor = 'k')
        plt.title(title)
        pos = graphviz_layout(G, prog = 'dot')
        nx.draw(G, pos, with_labels = False, arrows = False) #, node_size=600)
        edge_labels = nx.get_edge_attributes(G, 'sep')
        nx.draw_networkx_edge_labels(G, pos, edge_labels, font_color = 'red') #=formatted_edge_labels,font_color='red')
        
            

    def plot_scatter(self):
        if not self.traversed:
            self.traverse_instance()
        title = 'scatter - '+self.wcsp_basename

        #f, (ax_hard, ax_soft) = plt.subplots(2, 1, sharex=True)
        fig = plt.figure('scatter '+self.wcsp_filename, figsize = self.din_a4)
        (ax_hard, ax_soft) = fig.subplots(2, 1, sharex=True)

        arity = [a for a,c in self.arity_cost_count]
        cost =  [c for a,c in self.arity_cost_count]
        count = [self.arity_cost_count[a,c] for a,c in self.arity_cost_count]
        # broken axis: https://matplotlib.org/3.1.0/gallery/subplots_axes_and_figures/broken_axis.html

        # plot the same data on both axes
        ax_hard.scatter(x=arity, y=cost, s=count, alpha=0.5)
        ax_soft.scatter(x=arity, y=cost, s=count, alpha=0.5)

        soft_cost = set()
        for c in cost:
            if c < self.UB: soft_cost.add(c)
            assert c >= 0
            assert c <= self.UB
        # zoom-in / limit the view to different portions of the data
        ax_hard.set_ylim(self.UB-1, self.UB+1)  # outliers only
        ax_soft.set_ylim(0, max(soft_cost)*1.2)  # most of the data

        # hide the spines between ax_hard and ax_soft
        ax_hard.spines['bottom'].set_visible(False)
        ax_soft.spines['top'].set_visible(False)
        ax_soft.xaxis.set_major_locator(MaxNLocator(integer=True))
        ax_hard.yaxis.set_major_locator(MaxNLocator(integer=True))
        ax_soft.yaxis.set_major_locator(MaxNLocator(integer=True))
        ax_hard.tick_params(labeltop=False)  # don't put tick labels at the top
        ax_soft.xaxis.tick_bottom()
        ax_hard.xaxis.tick_top()
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            ax_hard.axes.yaxis.set_ticklabels(('', str(self.UB)+'\n(top\ncost)', ''))
        ax_soft.set_ylabel('cost')
        ax_soft.set_xlabel('arity')

        d = .015  # how big to make the diagonal lines in axes coordinates
        # arguments to pass to plot, just so we don't keep repeating them
        kwargs = dict(transform=ax_hard.transAxes, color='k', clip_on=False)
        ax_hard.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
        ax_hard.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

        kwargs.update(transform=ax_soft.transAxes)  # switch to the bottom axes
        ax_soft.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
        ax_soft.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal
        
        ax_hard.set_title(title)

    def plot_bar(self):
        if not self.traversed:
            self.traverse_instance()
        title = 'bar - '+self.wcsp_basename

        plt.figure('bar '+self.wcsp_filename)
        plt.title(title)

        arity_set = {a for a in self.arity_count}
        min_ary = min(arity_set)
        max_ary = max(arity_set)

        columns = [f'{a}-ary' for a in range(min_ary, max_ary+1)]
        rows = ['hard', 'mixt', 'soft', 'total']
        colors = ['crimson', 'orange', 'limegreen', 'white']

        data_rel = np.zeros((4, (max_ary+1 - min_ary)))
        #data_abs = np.zeros((4, (max_ary+1 - min_ary)))
        for a,h in self.arity_hardness_count:
            j = a - min_ary
            i = rows.index(h)
            data_rel[i,j] = 100 * self.arity_hardness_count[a,h] / self.num_cost_func
            #data_abs[i,j] = self.arity_hardness_count[a,h]
            data_rel[rows.index('total'),j] += data_rel[i,j]

        index = np.arange(len(columns)) + 0.3
        bar_width = 0.4        
        y_offset = np.zeros(len(columns)) # Initialize the vertical-offset for the stacked bar chart.

        # Plot bars and create text labels for the table
        cell_text = []
        for row in [3,2,1,0]:
            if rows[row] != 'total':
                plt.bar(index, data_rel[row], bar_width, bottom=y_offset, color=colors[row])
                y_offset = y_offset + data_rel[row]
            cell_text.insert(0, [f'{round(x, 3)}%' for x in data_rel[row]])
        # Reverse colors and text labels to display the last value at the top.
        #cell_text.reverse()

        # Add a table at the bottom of the axes
        plt.table(cellText=cell_text,
                        rowLabels=rows,
                        rowColours=colors,
                        cellColours=[[c]*len(columns) for c in colors],
                        colLabels=columns,
                        loc='bottom')

        # Adjust layout to make room for the table:
        plt.subplots_adjust(left=0.2, bottom=0.2)

        plt.ylabel(f"% functions")
        plt.xticks([])
        #plt.title('Dummy data')

        plt.figtext(0.5, 0.01, f'(share over {self.num_cost_func} functions in instance)', \
            wrap=True, horizontalalignment='center')



