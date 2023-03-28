import numpy as np
import matplotlib.pyplot as plt

#Lucas Schuurmans-Stekhoven
#Student No. 46363468

def find_all_paths(adj, nodes, start_node, end_node, path=[]):
    """ find all paths from start_node to 
        end_node in adj

        Parameters:
            adj(array): An np.inf array with size of (num_nodes, num_nodes)
            with edge functions located at [i,j] where an edge exists between
            i and j.
            nodes(array): Unique nodes within edges.
            start_node(int): Start node.
            end_node(int): End node.
            path(list): Empty list

        Returns:
            paths(array): All possible paths between start_node
            and end_node.

        """
    path = path + [start_node]
    if start_node == end_node:
        return [path]
    if start_node not in nodes or end_node not in nodes:
        return []
    paths = []
    neighbours = np.where(adj[start_node,:] != np.inf)
    for node in neighbours[0]:        
        if node not in path:
            extended_paths = find_all_paths(adj,
                                            nodes,
                                            node, 
                                            end_node, 
                                            path)
            for p in extended_paths:                
                paths.append(p)            
    return paths


# edges that are provided
edges = np.array([[0, 1, lambda N: N/100],
                  [0, 2, lambda N: 45],
                  [1, 3, lambda N: 45],
                  [2, 3, lambda N: N/100]])
                  

# edges that are provided
edges_modified = np.array([[0, 1, lambda N: N/100],
                  [0, 2, lambda N: 45],
                  [1, 3, lambda N: 45],
                  [2, 3, lambda N: N/100],
                  [1, 2, lambda N: 5]])


def create_network(edges):
    """ Creates an adjacency and nodes array.

    Parameters:
        edges(array): Array of edges (roads) with travel-time
        functions.

    Returns:
        adj(array): An np.inf array with size of (num_nodes, num_nodes)
        with edge functions located at [i,j] where an edge exists between
        i and j.
        nodes(array): Unique nodes within edges.
    """

    nodes = np.unique(edges[:,:2])
    num_nodes = len(nodes)
    adj_shape = (num_nodes, num_nodes)
    adj = np.full(adj_shape, np.inf, dtype='object')

    #below loop checks for node connections within edges.
    #then inserts trav_time_function at found connections. ie. [i,j]
    for i in range(num_nodes):
        for j in range(num_nodes):
            for l in edges:
                if i == l[0] and j == l[1]:
                    adj[i,j] = l[2]
    
    return adj, nodes


def calc_travtimes(adj, flows):
    """ Calculates travel times for each edge by inputting
    current flows for that edge within the edge travel-time
    function.

    Parameters:
        adj(array): An np.inf array with size of (num_nodes, num_nodes)
        with edge functions located at [i,j] where an edge exists between
        i and j.
        flows(array): Array with size (num_nodes, num_nodes) containing
        flow of vehicles on each edge.
    Returns:
        travtimes(array): Array with size (num_nodes, num_nodes) containing
        the travel time in minutes for each edge.
    """
    
    travtimes = np.full(adj.shape, np.inf, dtype='object')
    #below loop calculates travel times for each edge.
    for i in range(len(adj)):
        for j in range(len(adj)):
            if adj[i, j] != np.inf:
                function = adj[i,j]
                travel_time = function(flows[i,j])
                travtimes[i,j] = travel_time
        
    return travtimes


def find_shortest_path(travtimes, all_paths):

    """ Finds shortest path by calculating total travel time
    on each path

    Parameters:
        travtimes(array): Array with size (num_nodes, num_nodes) containing
        the travel time in minutes for each edge.
        all_paths(array): All possible paths within an edges array.

    Returns:
        sp(array): Shortest path found within all_paths array
        sp_index(int): Index location of shortest path within
        all_paths list.        
    """
    
    
    current_min_time = np.inf
    current_min_index = 0
    for index, path in enumerate(all_paths):
        total_time = 0
        #below loop calculates total travel time for path
        for j in range(len(path)-1):
            node1 = path[j]
            node2 = path[j+1]
            total_time += travtimes[node1, node2]
        if total_time < current_min_time:
            current_min_time = total_time
            current_min_index = index
            
    sp = all_paths[current_min_index]
    sp_index = current_min_index
             

    return sp, sp_index



def add_vehicle(sp, sp_index, flows, path_flows):

    """ Adds one vehicle to the shortest path

    Parameters:
        sp(array): Shortest path found within all_paths array
        sp_index(int): Index location of shortest path within
        all_paths.
        flows(array): Array with size (num_nodes, num_nodes) with
        number of vehicles in each edge.
        path_flows(array): Number of vehicles on each path found.

    Returns:
        flows(array): An updated flows array.
        path_flows(array): An updated path_flows_array.
    """

    for j in range(len(sp)-1):
        node1 = sp[j]
        node2 = sp[j+1]
        flows[node1,node2] += 1
        
    path_flows[sp_index] += 1

    return flows, path_flows


def calc_totaltime(flows, travtimes):

    """ Calculates total travel time for all edges by multiplying
    number of vehicles on the edge (flows) by the travel time on that edge.

    Parameters:
        flows(array): Array with size (num_nodes, num_nodes) with
        number of vehicles in each edge.
        travtimes(array): Travel time for each edge.

    Returns:
        total_time(int): Total travel time on all edges.    
    """

    total_time = 0
    num_nodes = len(flows)
    for i in range(num_nodes):
        for j in range(num_nodes):
            if travtimes[i,j] != np.inf:
                total_time += (flows[i,j] * travtimes[i,j])
    

    return total_time


def loading(num_veh, start_node, end_node, edges):

    """ Creates a totaltime series which is an accumulative sum
    of path travel times. One vehicle is added to the shortest path
    after every travel time calculation.

    Parameters:
        num_veh(int): Number of vehicles to be added to edges.
        start_node(int): Start node.
        end_node(int): End node.
        edges(array): Array consisting of multiple edges.

    Returns:
        totaltime_series(array): Accumulative sum of travel
        time.
    """
    
    #below block initializes required arrays for TTS calculation.
    adj, nodes = create_network(edges)
    flows = np.zeros_like(adj)
    all_paths = find_all_paths(adj, nodes, start_node, end_node)
    path_flows = np.zeros((len(all_paths), 1))
    totaltime_series = np.zeros(((num_veh),1))
    travtimes = calc_travtimes(adj, flows)

    #below loop calculates totaltime_series.
    for i in range(num_veh):
        sp, sp_index = find_shortest_path(travtimes, all_paths)
        flows, path_flows = add_vehicle(sp, sp_index, flows, path_flows)
        travtimes = calc_travtimes(adj, flows)
        total_time = calc_totaltime(flows, travtimes)
        totaltime_series[i] = total_time
        
    
    return totaltime_series



def main(num_veh, edges, edges_modified):

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
    TTS_edges = loading(num_veh,
                        start_node,
                        end_node,
                        edges)
    TTS_edges_modified = loading(num_veh,
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



