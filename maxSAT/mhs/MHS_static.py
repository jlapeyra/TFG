from ordered_set import OrderedSet
from pysat.examples.hitman import Hitman

def cost(h, wght_dict):
    return sum([wght_dict[b] for b in h])


class MHS_static:

    def __init__(self, weights, pop_mode=1, sort_mode='always', sort_order=1, performance={}):
        '''weights : dict(obj:num) '''
        self.C = [] # conjunt de conjunts, el problema
        self.all_weights = weights # tots els pesos
        self.weights = {} # pesos dels elements presents
        self.ub = 0 # upper bound del cost l'òptim
        self.h = set() # hitting set (òptim o subòptim)
        #self.optim = True # indica si self.h és òptim
        self.occurrences = {} # { x : count{ k∈C | x∈k } | ∃k∈C st x∈k }

        self.pop_mode = pop_mode
        self.sort_mode = sort_mode
        self.sort_order = sort_order
        self.performance = performance
        self.performance['mhs_backtracks'] = 0
        self.performance['mhs_calls'] = 0
        self.performance['mhs_calculations'] = 0 #nombre de vegades que s'ha calculat MHS (sense comptar quan l'optim està ja calculat)

    '''
    pop_mode fa referència al pop de cada crida recursiva de l'algoritme:
        ...
    sort_mode fa referència a l'ordre amb què es recorren els conjunts, concretament quan s'ordenen
        - 'never' -- cap 
        - 'once' -- s'ordenen una vegada, abans d'executar l'algorisme
        - 'always' -- s'ordenen cada vegada que es recorren
    sort_order fa referència a l'ordre amb què es recorren els conjunts, concretament amb quin criteri s'ordenen
        ...
    '''

    
    def hit(self, k):
        '''k : iter(object) -- set to hit'''
        k = set(k)
        
        pop_list = []
        for i in range(len(self.C)):
            if self.C[i].issubset(k): return
            if k.issubset(self.C[i]): pop_list.append(i)
        pop_list.reverse()
        for i in pop_list:
            for x in self.C[i]:
                self.occurrences[x] -= 1
            self.C.pop(i)
            
        self.C.append(k)
        for x in k:
            if x in self.occurrences:
                self.occurrences[x] += 1
            else:
                self.occurrences[x] = 1
                self.weights[x] = self.all_weights[x]

        self._update_ub(k)

    def _update_ub(self, k):
        self.ub += min([self.weights[x] for x in k])
 
    
    def _pop_priority(self, k, occurrences):
        if self.pop_mode == 1: return -len(k), min([occurrences[x] for x in k]), min([self.weights[x] for x in k])
        if self.pop_mode == 2: return -len(k), sum([occurrences[x] for x in k]), sum([self.weights[x] for x in k])
        if self.pop_mode == 3: return sum([occurrences[x]*self.weights[x] for x in k])/len(k)
    
    def _pop(self, C, occurrences):
        '''C : iter(iter(obj))
           occurrences : dict(obj:int)
           pops the element of C with highest priority'''
        max_prio, imax = max([(self._pop_priority(C[i], occurrences), i) for i in range(len(C))])
        return C.pop(imax)
    
    def _sort_priority(self, x, occurrences):
        if self.sort_order == 1: return occurrences[x], -self.weights[x]
        if self.sort_order == 2: return occurrences[x]/self.weights[x]

    def _sort(self, k, occurrences):
        '''k : iter(obj)
           occurrences : dict(obj:int)
           sorts k by deacreasing order of priority'''
        pairs = [(self._sort_priority(x, occurrences), x) for x in k]
        pairs.sort(reverse=True)
        return [p[1] for p in pairs]
            

    def _get(self, C, ub, occurrences):
        '''
        C : iter(iter(obj)) -- problem
        ub : num -- upper bound of the cost
        occurrences : dict(obj:int) -- occurrences[x] is the number of times that x appears in C

        return cost, h
        h -- minimum hitting set of C
        cost -- cost of h
        '''
        if len(C) == 0: return 0, set()
        self.performance['mhs_backtracks'] += 1
        
        k = self._pop(C, occurrences)
        for x in k: occurrences[x] -= 1
        if self.sort_mode == 'always':
            k = self._sort(k, occurrences)
        
        h = set()
        
        # oerdernar k segons el següent criteri:
        # - Primer els presents a la solució sub-òptima (basada en l'últim òptim), self.h
        # - cost gran i que surtin en molts conjunts
        for u in k:
            cost_u = self.weights[u]
            if cost_u < ub:
                C_ = []
                occ = occurrences.copy()
                for k_ in C: # O(len(C))
                    if not u in k_: # O(1) -- k_ is a set
                        C_.append(k_) # O(1) -- C_ is a list
                    else:
                        for x in k_: occ[x] -= 1
                        
                cost_h_, h_ = self._get(C_, ub - cost_u, occ)
                if cost_u + cost_h_ < ub:
                    h_.add(u)
                    h = h_
                    ub = cost_u + cost_h_
        return ub, h

    def get(self):
        '''Computes the minimum hitting set'''
        self.performance['mhs_calls'] += 1
        
        '''if self.optim:
            #print('previously solved')
            return self.h
        else:
            #print('llarg')
            self.optim = True'''

        if self.sort_mode == 'once':
            C = []
            for c in self.C:
                C.append(OrderedSet(self._sort(c, self.occurrences.copy())))
        else:
            C = self.C.copy()
        
        ub, h = self._get(C, self.ub, self.occurrences.copy())
        if ub < self.ub:
            self.ub = ub
            self.h = h
        self.performance['mhs_calculations'] += 1
        return self.h

    def getSubOptimal(self):
        return self.h

'''
if __name__ == '__main__':
    import random as r
    n_var = r.randrange(10, 40)
    max_size_set = r.randrange(2, 20)
    wght = {}
    for i in range(n_var):
        wght[i] = r.randrange(1, 300)
    hitman = Hitman()
    mhs = MHS_static(wght)
    num_sets = r.randrange(10, 30)

    for i in range(num_sets):
        size_set = r.randrange(1, max_size_set+1)
        k = set()
        for j in range(size_set):
            x = r.randrange(0, n_var)
            x = x - r.randrange(0, x+1)
            k.add(x)

        if True:#i+1 == num_sets:
            hitman.hit(k, weights=wght)
            r1 = hitman.get()
            c1 = cost(r1, wght)

            mhs.hit(k)
            r2 = mhs.get()
            c2 = cost(r2, wght)

            if c1 == c2:
                print('OK', r1, c1, r2, c2)
            else:
                print('Error:', r1, c1, r2, c2)
'''
