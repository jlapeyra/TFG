from ordered_set import OrderedSet
from mhs.MHS_static import *

class MHS_dynamic(MHS_static):

    # v2+v3 (nomenclatura antiga)

    def __init__(self, weights, pop_mode=1, sort_mode='always', sort_order=1, \
        occset=True, performance={}):
        '''weights : dict(obj:num) '''
        super().__init__(weights, pop_mode=pop_mode, sort_mode=sort_mode, sort_order=sort_order, performance=performance)
        self.C_idx = set()
        self.occset = occset
        if self.occset: self.occurrences_set = {} # { x : { k_idx ∈ C_idx | x ∈ k } | ∃k_idx∈C_idx st x∈k }
        
    
    def hit(self, k):
        '''k : iter(object) -- set to hit'''
        k = set(k)

        # no subsets among them
        pop_set = set()
        for i in self.C_idx:
            k2 = self.C[i]
            if k2.issubset(k): return
            if k.issubset(k2): pop_set.add(i)
        self.C_idx.difference_update(pop_set)

        # add k
        k_idx = len(self.C)
        self.C_idx.add(k_idx)
        self.C.append(k)

        # Update occurrences & weigths
        for x in k:
            if x in self.occurrences:
                self.occurrences[x] += 1
                if self.occset: self.occurrences_set[x].add(k_idx)
            else:
                self.occurrences[x] = 1
                if self.occset: self.occurrences_set[x] = {k_idx}
                self.weights[x] = self.all_weights[x]
        
        self._update_ub(k)
    

    def _pop(self):
        max_prio, imax = max([(self._pop_priority(self.C[i], self.occurrences), i) for i in self.C_idx])
        self.C_idx.remove(imax)
        return imax
    
    def _propagate(self, u):
        if self.occset:
            self.C_idx.difference_update(self.occurrences_set[u])
            for i in self.occurrences_set[u]: 
                for x in self.C[i]:
                    if x != u:
                        self.occurrences[x] -= 1
                        self.occurrences_set[x].remove(i)
        else:
            remove_set = set()
            for i in self.C_idx: 
                if u in self.C[i]: 
                    remove_set.add(i)
                    for x in self.C[i]: self.occurrences[x] -= 1
            self.C_idx.difference_update(remove_set)
            return remove_set
    
    def _unpropagate(self, u, remove_set=None):
        if self.occset:
            self.C_idx.update(self.occurrences_set[u])
            for i in self.occurrences_set[u]: 
                for x in self.C[i]:
                    if x != u:
                        self.occurrences[x] += 1
                        self.occurrences_set[x].add(i)
        else:
            self.C_idx.update(remove_set)
            for i in remove_set: 
                for x in self.C[i]: self.occurrences[x] += 1

        
    def _get(self, ub):
        '''
        C : iter(iter(obj)) -- problem
        ub : num -- upper bound of the cost
        occurrences : dict(obj:int) -- occurrences[x] is the number of times that x appears in C

        return cost, h
        h -- minimum hitting set of C
        cost -- cost of h
        '''
        #print([self.C[i] for i in C_idx])
        if len(self.C_idx) == 0: return 0, set()
        self.performance['mhs_backtracks'] += 1
        
        k_idx = self._pop()
        k = self.C[k_idx]
        for x in k: 
            self.occurrences[x] -= 1
            if self.occset: self.occurrences_set[x].remove(k_idx)
        if self.sort_mode == 'always':
            k = self._sort(k, self.occurrences)
        
        h = set()

        #print(k)
        for u in k:
            cost_u = self.weights[u]
            if cost_u < ub:
                info = self._propagate(u)

                cost_h_, h_ = self._get(ub - cost_u)

                if cost_u + cost_h_ < ub:
                    h_.add(u)
                    h = h_
                    ub = cost_u + cost_h_
                self._unpropagate(u, info)
        
        for x in k:
            self.occurrences[x] += 1
            if self.occset: self.occurrences_set[x].add(k_idx)
        self.C_idx.add(k_idx)
        
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
                C.append(OrderedSet(self._sort(c, self.occurrences)))
            self.C = C

        #print(C_idx)

        ub, h = self._get(self.ub)
        
        if ub < self.ub:
            self.ub = ub
            self.h = h
        self.performance['mhs_calculations'] += 1
        return self.h


if __name__ == '__main__':
    n_var = r.randrange(10, 40)
    max_size_set = r.randrange(2, 20)
    wght = {}
    for i in range(n_var):
        wght[i] = r.randrange(1, 300)
    hitman = Hitman()
    mhs = MHS_dynamic(wght)
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

