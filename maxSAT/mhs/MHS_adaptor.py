import sys
from mhs.MHS_static import *
from mhs.MHS_dynamic import *
from mhs.MHS_cplex import *
from pysat.examples.hitman import Hitman

mhs_solver_names = ['hitman', 'cplex', 'static', 'dynamic', 'dynamic+']

class MHS_adaptor:

    def __init__(self, weights:dict, solver='hitman', sets_to_hit=[], \
        pop_mode=1, sort_mode='always', sort_order=1, performance={}):
        '''
        solver : 
            - hitman   -- solver pysat.examples.hitman
            - cplex    -- solver basat en programació lineal
            - static   -- solver recursiu, versió estàtica (cada recusrivitat construeix les dades)
            - dynamic  -- solver recursiu, versió dinàmica (les mateixes dades canvien a cada recursivitat),
            - dynamic+ -- idem, amb occurrences_set
        '''
        self.type = solver
        self.weights = weights
        if self.type == 'hitman':
            self.mhs = Hitman()
        elif self.type == 'cplex':
            self.mhs = MHS_cplex(weights)
        elif self.type == 'static':
            self.mhs = MHS_static(weights, pop_mode=pop_mode, sort_mode=sort_mode, sort_order=sort_order, performance=performance)
        elif self.type == 'dynamic':
            self.mhs = MHS_dynamic(weights, occset=False, pop_mode=pop_mode, sort_mode=sort_mode, sort_order=sort_order, performance=performance)
        elif self.type == 'dynamic+':
            self.mhs = MHS_dynamic(weights, occset=True, pop_mode=pop_mode, sort_mode=sort_mode, sort_order=sort_order, performance=performance)
        else:
            raise Exception("Wrong argument\n" +
                f"\t{self.type} was not recognized as a MHS solver\n" + \
                "\tTry one of these: hitman, cplex, static, dynamic, dynamic+")

        for k in sets_to_hit:
            self.hit(k)

    num_sets = 0
    h = set()
    optim = True

    def _update_hitting_set(self, k): #actualitza h perquè sigui hitting set
        if k.isdisjoint(self.h):
            (minw, minw_elem) = min([(self.weights[x], x) for x in k])
            #self.ub += minw
            self.h.add(minw_elem)
            self.optim = False

    def hit(self, k): # add a set to hit
        if self.type=='hitman': self.mhs.hit(k, self.weights)
        else:                   self.mhs.hit(k)
        self.num_sets += 1
        self._update_hitting_set(set(k))

    def getMHS(self): # compute Minimum Hitting Set
        self.h = set(self.mhs.get())
        return self.h

    def getHS(self): # get Hitting Set
        return self.h

    def getCost(self):
        '''get cots of the last computed HS or MHS'''
        return cost(self.h, self.weights)



def check_HS(C, h):
    #print('checking Hitting Set...')
    h = set(h)
    for k in C:
        assert(not h.isdisjoint(k))

def check_MHS(C, h, weights):
    check_HS(C, h)
    #print('checking Min Hitting Set...')
    hitman = Hitman(bootstrap_with=C, weights=weights)
    h2 = hitman.get()
    #print(h)
    #print(h2)
    assert(cost(h, weights) == cost(h2, weights))
    #print('Min Hitting Set OK')


'''
if __name__ == '__main__':
    import random as r

    for iter in range(30):
        n_var = r.randrange(50, 700)
        max_size_set = r.randrange(3, 100)
        print(n_var, 'variables')
        print('Sets of up to', max_size_set, 'variables')
        wght = {}
        for i in range(n_var):
            wght[i] = r.randrange(1, 300)

        mhs = MHS_adaptor(wght, 'cplex')
        K = []

        num_sets = r.randrange(20, 200)
        for i in range(num_sets):
            size_set = r.randrange(1, max_size_set+1)
            k = {r.randrange(0, n_var) for j in range(size_set)}
            mhs.hit(k)
            K.append(k)
            print("k", k)

        check_HS(K, mhs.getHS())
        print('hs', mhs.getHS())
        print('HS OK')

        h = mhs.getMHS()
        print('mhs', h)
        check_HS(K, h)
        check_MHS(K, h, wght)
        print('MHS OK')
        
        print()
'''