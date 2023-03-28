import numpy as np
import matplotlib.pyplot as plt



def find_all_paths(adj, nodes, start_node, end_node, path=[]):
    """ find all paths from start_node to 
        end_node in adj """
    
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
    """
    """
    nodes = np.array([0,1,2,3], dtype='object')
    num_nodes = len(nodes)
    adj_shape = (num_nodes, num_nodes)
    adj = np.full(adj_shape, np.inf, dtype='object')
    nodes = np.array([0,1,2,3], dtype='object')

    for i in range(len(edges)):
        for j in range(len(edges)):
            for l in edges:
                if i == l[0] and j == l[1]:
                    adj[i,j] = l[2]
    
    return adj, nodes


def calc_travtimes(adj, flows):
    travtimes = np.zeros_like(adj)
    for i in range(len(adj)):
        for j in range(len(adj)):
            if adj[i, j] != np.inf:
                function = adj[i,j]
                travel_time = function(flows[i,j])
                travtimes[i,j] = travel_time
        
    return travtimes


def find_shortest_path(travtimes, all_paths):
    
    current_min_time = np.inf
    current_min_index = 1
    for index, path in enumerate(all_paths):
        total_time = 0
        for j in range(len(path)-1):
            node1 = path[j]
            node2 = path[j+1]
            total_time += travtimes[node1, node2]
        if total_time < current_min_time:
            current_min_time = total_time
            current_min_index = index
            
    #print(current_min_index)
    

    return all_paths[current_min_index], current_min_index



def add_vehicle(sp, sp_index, flows, path_flows):

    for j in range(len(sp)-1):
        node1 = sp[j]
        node2 = sp[j+1]
        flows[node1,node2] += 1
        
    path_flows[sp_index] += 1

    return flows, path_flows


def calc_totaltime(flows, travtimes):

    total_time = 0
    for i in range(len(flows)):
        for j in range(len(flows)):
            if travtimes[i,j] != np.inf:
                total_time += (flows[i,j] * travtimes[i,j])
    

    return total_time


def loading(num_veh, start_node, end_node, edges):

    adj, nodes = create_network(edges)
    flows = np.zeros_like(adj)
    all_paths = find_all_paths(adj, nodes, start_node, end_node)
    path_flows = np.zeros((len(all_paths), 1))
    totaltime_series = np.zeros(num_veh)

    for i in range(num_veh):
        
        travtimes = calc_travtimes(adj, flows)
        sp, sp_index = find_shortest_path(travtimes, all_paths)
        flows, path_flows = add_vehicle(sp, sp_index, flows, path_flows)
        total_time = calc_totaltime(flows, travtimes)
        totaltime_series[i] = total_time
        
    
    return totaltime_series



def main(num_veh, edges, edges_modified):

    start_node = 0
    end_node = 3
    adj, nodes = create_network(edges)
    flows = np.zeros_like(adj)
    travtimes = calc_travtimes(adj, flows)
    all_paths = find_all_paths(adj, nodes, 0,3)
    path_flows = np.zeros((len(all_paths), 1))
    sp, sp_index = find_shortest_path(travtimes, all_paths)
    flows, path_flows = add_vehicle(sp, sp_index, flows, path_flows)
    TSS = calc_totaltime(flows, travtimes)
    totaltime_series_edges = loading(num_veh, start_node, end_node, edges)
    totaltime_series_edges_modified = loading(num_veh, start_node, end_node, edges_modified)

    fig, ax = plt.subplots()

    ax.plot(totaltime_series_edges, label='before')
    ax.plot(totaltime_series_edges_modified, label='after')
    ax.set_xlabel("number of vehicle")
    ax.set_ylabel("total time spent [veh.min]")
    ax.legend()
    
                
    fig.show()

    return totaltime_series_edges, totaltime_series_edges_modified
    


    
if __name__ == "__main__":
    main()
