import numpy as np
import matplotlib.pyplot as plt


#Lucas Schuurmans-Stekhoven
#Student No. 46363468

class Road(object):
    
    """ Creates an instance of Road."""
    
    def __init__(self, src, snk, travtime_func):

        """ Initializes Road object.

        Parameters:
            src(int): Start node
            snk(int): End node
            travtime_func(function): Function associated with road.
            
        Returns:
            None.
        """
        
        self._numveh = 0
        self.src = src
        self.snk = snk
        self._travtime_func = travtime_func

    def add_vehicle(self):
        
        """Adds 1 vehicle to numveh."""
        
        self._numveh += 1

    def get_vehicle(self):
        
        """(int) Return number of vehicles."""
        
        return self._numveh

    def calc_travtime(self):

        """(float) Returns travel time on road."""
        
        return self._travtime_func(self._numveh)


class Network(object):

    """Creates an instance of Network which is a list of Road objects."""

    def __init__(self, roads):
        
        """Initializes network by creating adj, nodes and travtimes arrays.

        Parameters:
            roads(list): List of Road objects.

        Returns:
            None.
        """
        
        all_nodes = []
        self.roads = roads
        
        for i in roads:
            all_nodes.append((i.src, i.snk))
        nodes = np.unique(all_nodes)
        
        num_nodes = len(nodes)
        adj_shape = (num_nodes, num_nodes)
        adj = np.full(adj_shape, np.inf, dtype='object')
        travtimes = np.full_like(adj, np.inf)
        
        #below loop inputs Road objects into adjacency array
        for i in range(num_nodes):
            for j in range(num_nodes):
                for l in roads:
                    if i == l.src and j == l.snk:
                        adj[i,j] = l
                        
        self._adj = adj
        self._nodes = nodes
        self._travtimes = travtimes


    def calc_travtimes(self):

        """Calculates travtimes for each Road."""
        
        for i in range(len(self._adj)):
            for j in range(len(self._adj)):
                if self._adj[i, j] != np.inf:
                    travel_time = self._adj[i,j].calc_travtime()
                    self._travtimes[i,j] = travel_time
                        

    def find_all_paths(self, start_node, end_node, path=[]):

        """ find all paths from start_node to 
        end_node in adj"""
    
        path = path + [start_node]
        if start_node == end_node:
            return [path]
        if start_node not in self._nodes or end_node not in self._nodes:
            return []
        paths = []
        neighbours = np.where(self._adj[start_node,:] != np.inf)
        for node in neighbours[0]:        
            if node not in path:
                extended_paths = self.find_all_paths(node, end_node, path)
                
                for p in extended_paths:                
                    paths.append(p)
        
        return paths
            

    def find_shortest_path(self, all_paths):

        """ Finds the shortest path within the network.

        Parameters:
            all_paths(array): All possible paths within the network.

        Returns:
            sp(list): Shortest path within the network.
            sp_index(int): Index location of shortest path within
            all_paths list.
        """

        current_min_time = np.inf
        current_min_index = 0
        for index, path in enumerate(all_paths):
            total_time = 0
            #below loop calculates travel time for the path.
            for j in range(len(path)-1):
                node1 = path[j]
                node2 = path[j+1]
                total_time += self._travtimes[node1, node2]
            if total_time < current_min_time:
                current_min_time = total_time
                current_min_index = index
                
        sp = all_paths[current_min_index]
        sp_index = current_min_index

        return sp, sp_index


    def add_vehicle(self, sp, sp_index, path_flows):

        """Adds 1 vehicle to each road within the shortest path list.

        Parameters:
            sp(list): Shortest path within the network.
            sp_index(int): Index location of shortest path within
            all_paths list.
        Returns:
            path_flows(array): Number of vehicles on each path within
            all_paths list.

        """
        for i in range(len(sp)-1):
            node1 = sp[i]
            node2 = sp[i+1]
            self._adj[node1, node2].add_vehicle()
        
        path_flows[sp_index] += 1

        return path_flows



    def calc_totaltime(self):

        """(float) Returns sum of travel times on each road as a function
        of the number of vehicles on that road."""

        total_time = 0
        for road in self.roads:
            for i in range(len(self._nodes)):
                for j in range(len(self._nodes)):
                    if road.src == i and road.snk == j:
                        total_time += self._travtimes[i,j]*road.get_vehicle()
            

        return total_time



def loading_object(num_veh, start_node, end_node, edges):

    """ Creates a totaltime series which is an accumulative sum
    of Network travel times. One vehicle is added to the shortest path
    after every travel time calculation.

    Parameters:
        num_veh(int): Number of vehicles to be added to the network.
        start_node(int): Start node.
        end_node(int): End node.
        edges(array): Array of edges.

    Returns:
        totaltime_series(array): Accumulative sum of travel times
        within the network as vehicles are added.

    """

    roads = []
    #below loop creates list of road objects from edges.
    for edge in edges:
        src, snk, function = edge
        road = Road(src, snk, function)
        roads.append(road)

    #below block initializes and creates required arrays for next loop.
    ntwrk = Network(roads)
    all_paths = ntwrk.find_all_paths(start_node, end_node, path=[])
    path_flows = np.zeros(len(all_paths))
    totaltime_series = np.zeros((num_veh,1))
    ntwrk.calc_travtimes()
    
    #below loop creates the totaltime_series array.
    for i in range(num_veh):
        
        sp, sp_index = ntwrk.find_shortest_path(all_paths)
        path_flows = ntwrk.add_vehicle(sp, sp_index, path_flows)
        ntwrk.calc_travtimes()
        total_time = ntwrk.calc_totaltime()        
        totaltime_series[i] = total_time
    
    return totaltime_series




def main_object(num_veh, edges, edges_modified):

    """ Calculates the totaltime_series for edges and edges_modified
    and both are graphed on the same axis for comparison.

    Parameters:
        num_veh(int): Number of vehicles to be added to edges
        edges(array): Array of edges
        edges_modified(array): Array of edges with an added edge.

    Returns:
        None.
    """
    
    start_node = 0
    end_node = 3
    TTS_edges = loading_object(num_veh,
                               start_node,
                               end_node,
                               edges)
    TTS_edges_modified = loading_object(num_veh,
                                        start_node,
                                        end_node,
                                        edges_modified)

    fig, ax = plt.subplots()
    ax.plot(TTS_edges, label='before')
    ax.plot(TTS_edges_modified, label='after')
    ax.set_xlabel("number of vehicles")
    ax.set_ylabel("total time spent [veh.min]")
    ax.legend()

    fig.show()

edges = np.array([[0, 1, lambda N: N/100],
                  [0, 2, lambda N: 45],
                  [1, 3, lambda N: 45],
                  [2, 3, lambda N: N/100]])

start_node=0
end_node=3
num_veh=4000

roads = []
#below loop creates list of road objects from edges.
for edge in edges:
    src, snk, function = edge
    road = Road(src, snk, function)
    roads.append(road)

#below block initializes and creates required arrays for next loop.
ntwrk = Network(roads)
all_paths = ntwrk.find_all_paths(start_node, end_node, path=[])
path_flows = np.zeros(len(all_paths))
totaltime_series = np.zeros((num_veh,1))
ntwrk.calc_travtimes()

#below loop creates the totaltime_series array.
for i in range(num_veh):
    
    sp, sp_index = ntwrk.find_shortest_path(all_paths)
    path_flows = ntwrk.add_vehicle(sp, sp_index, path_flows)
    ntwrk.calc_travtimes()
    total_time = ntwrk.calc_totaltime()        
    totaltime_series[i] = total_time

    

