import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Lucas Schuurmans-Stekhoven
# Student No. 46363468

STATES = ['QLD','NSW','VIC','TAS','SA']
STEP_TIME_DAY = np.timedelta64(1, 'D')
STEP_TIME_MINUTES = np.timedelta64(30, 'm')
START_DATE = '2015-01-01T00:30'
END_DATE = '2021-01-01T00:00'
STUDY_YEARS = ['2015','2016','2017','2018','2019','2020']
DAY_STEPS = 48


def import_data():
    """ Imports date, grid demand, and pricing data for
    Australia's electricity network.

    Parameters:
        None

    Returns:
        regions(dictionary): Australian states including National Energy Market(NEM)
        with assigned integers.
        date_data(array): Half hourly time data associated with price
        and grid demand for each state.
        demand(array): Half-hourly grid demand for each state in MW.
        price(array): Half-hourly demand price for each state in $/MWh.
    """
    
    regions = {'QLD':0,'NSW':1,'VIC':2,'TAS':3,'SA':4, 'NEM':5}
    for region in regions:
        column = regions[region]
        if region != 'NEM':
            read_data = pd.read_csv(region+'.csv')
            rawdata = np.array(read_data)
            nrows = rawdata[:,0].shape
            data_shape = (nrows[0], len(STATES))
            demand_data_shape = (nrows[0], len(STATES)+1)
                
        if column < 1:
            date_data = np.zeros(data_shape, dtype='datetime64[m]')
            demand = np.zeros(demand_data_shape)
            price = np.zeros(data_shape)
            date_data[:, column] = np.array(rawdata[:, 1], dtype='datetime64[m]')
            demand[:, column] = np.array(rawdata[:, 2], dtype=float) * 10**-3
            price[:, column] = np.array(rawdata[:, 3], dtype=float)
            
        if column < 5:
            date_data[:, column] = np.array(rawdata[:, 1], dtype='datetime64[m]')
            demand[:, column] = np.array(rawdata[:, 2], dtype=float) * 10**-3
            price[:, column] = np.array(rawdata[:, 3], dtype=float)

        if column == 5:
            nem = np.zeros(nrows[0])
            for row in range(nem.size):
                nem[row] = np.sum(demand[row, :])
            nem.reshape(nem.size, 1)
            demand[:, column] = nem[:]

                 

    return regions, date_data, demand, price


def check_date(date_data):
    """ Takes in data_data and ensures each date data point matches with
    an array of reference dates by performing a number of tests.

    Parameters:
        data_data(array): Array of date data for each state in half-hourly
        time steps.

    Returns:
        ref_date(array): Array of reference dates.
        date_good(boolean): Returns true if all date tests pass.
        Returns false if any date test fails.
    """
    
    ref_date = np.arange(START_DATE,END_DATE,STEP_TIME_MINUTES, dtype='datetime64[m]')
    nrows = date_data.shape[0]
    ncolumns = date_data.shape[1]
    error_states = []
    count = 0
    
    while count == 0:
            
        if len(ref_date) > nrows:
            print('Warning! Too few date-time entries in data!')
            date_good = False
            count += 1
        elif len(ref_date) < nrows:
            print('Warning! Too many date-time entries in data!')
            date_good = False
            count += 1
        else:
            time_steps = np.diff(date_data, axis=0)
            min_time_step = np.min(np.array(time_steps, dtype=int))
            max_time_step = np.max(np.array(time_steps, dtype=int))
            if min_time_step != 30 or max_time_step != 30:
                print(f'Warning! Timestep varies from {min_time_step} to {max_time_step} minutes')
                date_good = False
                count += 1
            else:
                column = 0
                for state in STATES:
                    difference = date_data[:, column] == ref_date
                    if False in difference:
                        error_states.append(state)
                    column += 1
                    
                if error_states != []:
                    print(f'Warning! Datetime errors found in {error_states}')
                    date_good = False
                    count += 1
                else:
                    print('Datetime data from all states aligns with reference datetime')
                    date_good = True
                    count += 1

    return ref_date, date_good
    

def plot_box(demand, price):
    """ Creates two plots, half-hourly demand in GW against
    region, and half-hourly price in $/MWh against region.

        Parameters:
            demand(array): Half-hourly grid demand for each region in MW.
            price(array): Half-hourly price in $/MWh for each region.

        Returns:
            None.
    """
    
    regions = import_data()[0]
    fig, ax = plt.subplots(1,2)
    ax[0].set_xticklabels(regions)
    ax[0].boxplot(demand)
    ax[0].set_xlabel('Region')
    ax[0].set_ylabel('Half-hourly demand, GW')
    ax[1].set_xticklabels(STATES)
    ax[1].boxplot(price)
    ax[1].set_xlabel('Region')
    ax[1].set_ylabel('Half-hourly price, $/MWh')
    ax[1].set_yscale('log')
  
    fig.tight_layout()
    fig.show()


def plot_scatter(demand, price):
    """ Creates two scatter plots, half-hourly demand in GW
    against half-hourly price in $/MWh and half hourly demand
    normalized by the mean against half hourly price in $/MWh.

    Parameters:
        demand(array): Half-hourly grid demand for each region in MW.
        price(array): Half-hourly price in $/MWh for each region.

    Returns:
        None
    """
    
    markers = ['o', 's', 'd', '>', '*']
    fig, ax = plt.subplots(1,2)
    for i in range(len(markers)):
        ax[0].scatter(demand[:,i], price[:,i], marker=markers[i])
    ax[0].set_xlabel('Half-hourly demand, GW')
    ax[0].set_ylabel('Half-hourly price, $/MWh')

    for i in range(len(markers)):
        state_mean = np.mean(demand[:,i])
        ax[1].scatter(demand[:,i]/state_mean, price[:,i], marker=markers[i], label=STATES[i])
    ax[1].set_xlabel('Half-hourly demand, normalized by mean')
    ax[1].set_ylabel('Half-hourly price, $/MWh')
    ax[1].legend()
    
    fig.show()


def analyse_data(ref_date, demand):
    """ Function utilises demand data to create new arrays consisting
    of additional useful data including gw_d, diff_d, reldiff_d

    Parameters:
        ref_date(array): Datetime array of study time period.
        demand(array): Half-hourly grid demand for each region in MW.

    Returns:
        date_d(array): Datetime array of study time period
        with time steps of 1 day.
        gwh_d(array): GWh per day for each region
        diff_d(array): Difference in demand in GW for each
        day in each region.
        reldiff_d(array): Relative difference in demand in GW
        for each day in each region.
    """
    
    
    date_d = np.arange(START_DATE,END_DATE, STEP_TIME_DAY, dtype='datetime64[D]')
    daily_shape = (date_d.shape[0], demand.shape[1])
    gw_d = np.zeros(daily_shape)
    diff_d = np.zeros(daily_shape)
    reldiff_d = np.zeros(daily_shape)
    first_day = 1
    for day in range(0, len(date_d)):
        day_slice = slice(day*DAY_STEPS-1+first_day, (day+1)*DAY_STEPS-1)
        mean_demand = np.mean(demand[day_slice], axis=0)
        gw_d[day, :] = np.sum(demand[day_slice, :], axis=0)
        diff_d[day, :] = np.max(demand[day_slice, :], axis=0) - np.min(demand[day_slice, :], axis=0)
        reldiff_d[day, :] = diff_d[day, :] / mean_demand
        first_day = 0

    gwh_d = gw_d/2

    return date_d, gwh_d, diff_d, reldiff_d

    
def plot_timeseries(date_d, gwh_d, reldiff_d, state2plot):
    """ This function creates a figure with two line plots for
    the specified region.

    Parameters:
        date_d(array): Datetime array with time steps of 1 day.
        gwh_d(array): GWh per day for each region
        reldiff_d(array): Relative difference in demand in GW
        for each day in each region.
        state2plot(str): The state that will be plotted.

    Returns:
        None.
    """
    
    
    regions = {'QLD':0,'NSW':1,'VIC':2,'TAS':3,'SA':4, 'NEM':5}    
    fig, ax = plt.subplots(2,1)
    ax[0].plot(date_d, gwh_d[:, regions[state2plot]])
    ax[0].set_ylabel('Daily consumption (GWh)')
    ax[1].set_ylabel('Max daily diff: % of mean daily demand')
    ax[1].plot(date_d, reldiff_d[:, regions[state2plot]]*100)
    ax[0].set_xticklabels('')

    fig.suptitle(f'Daily grid consumption and maximum % daily differential for {state2plot} 2015-2020. Source: AEMO')
    fig.show()
   

def main():
    """ Takes the returns from each function previously listed and
    utilises the data to create additional useful data which is then
    printed. Lastly the function requests a state to plot and plots
    the timeseries.

    Parameters:
        None.

    Returns:
        None.
    """

    regions, date_data, demand, price = import_data()
    ref_date, date_good = check_date(date_data)
    date_d, gwh_d, diff_d, reldiff_d = analyse_data(ref_date, demand)
    gw_d = gwh_d*2
    states = np.array(STATES)

    mean_price = np.mean(price, axis=0)
    max_price = np.max(mean_price)
    max_state = states[mean_price == max_price]
    min_price = np.min(mean_price)
    min_state = states[mean_price == min_price]
    mean_demand = np.mean(demand[:,:5], axis=0)
    max_m_demand = np.max(mean_demand)
    max_d_state = states[mean_demand == max_m_demand]
    min_m_demand = np.min(mean_demand)
    min_d_state = states[mean_demand == min_m_demand]

    min_diff_d = np.min(diff_d[:, :5], axis=0)
    min_diff_d_abs = np.min(diff_d[:, :5])
    max_diff_d = np.max(diff_d[:, :5], axis=0)
    max_diff_d_abs = np.max(diff_d[:, :5])
    min_diff_state = states[min_diff_d_abs == min_diff_d]
    max_diff_state = states[max_diff_d_abs == max_diff_d]

    min_reldiff_d = np.min(reldiff_d[:, :5], axis=0)
    min_reldiff_d_abs = np.min(reldiff_d[:, :5])
    max_reldiff_d = np.max(reldiff_d[:, :5], axis=0)
    max_reldiff_d_abs = np.max(reldiff_d[:, :5])
    min_reldiff_d_state = states[min_reldiff_d_abs == min_reldiff_d]
    max_reldiff_d_state = states[max_reldiff_d_abs == max_reldiff_d]

    yearly_mean = np.zeros(6)
    year1_slice = slice(0,365)
    year2_slice = slice(365,731)
    year3_slice = slice(731,1096)
    year4_slice = slice(1096,1461)
    year5_slice = slice(1461,1826)
    year6_slice = slice(1826, 2192)
    year_slices = np.stack([year1_slice, year2_slice, year3_slice, year4_slice, year5_slice, year6_slice])
    study_years = np.array(STUDY_YEARS)
    year = 0

    for year_slice in year_slices:
        yearly_mean[year] = np.mean(reldiff_d[year_slice, -1])
        year += 1

    min_mean_reldiff_y = np.min(yearly_mean)
    max_mean_reldiff_y = np.max(yearly_mean)
    min_reldiff_d_NEM_year = study_years[min_mean_reldiff_y == yearly_mean]
    max_reldiff_d_NEM_year = study_years[max_mean_reldiff_y == yearly_mean]


    print(f'{max_state[0]} had the highest mean power price, ${max_price:.4} per MWh, and'
          f' {min_state[0]} had the lowest mean power price, ${min_price:.4} per MWh')
    print(f'Mean half-hourly demand varied from a minimum of {min_m_demand:.3} GW in {min_d_state[0]} to'
          f' a maximum of {max_m_demand:.3} GW in {max_d_state[0]}')
    print(f'Range in grid demand varied from a minimum of {min_diff_d_abs:.2}'
          f' GW in {min_diff_state[0]} to a maximum of {max_diff_d_abs:.3} GW in {max_diff_state[0]}')
    print(f'Relative range in grid demand over a single day varied from a minimum of'
          f' {min_reldiff_d_abs*100:.4}% in {min_reldiff_d_state[0]} to a maximum of {max_reldiff_d_abs*100:.5}% in {max_reldiff_d_state[0]}')
    print(f'Over the NEM for the entire study period, {max_reldiff_d_NEM_year[0]} was the year with the highest and {min_reldiff_d_NEM_year[0]}'
          f' was the year the lowest mean relative daily differential in grid demand: {max_mean_reldiff_y*100:.4}% and {min_mean_reldiff_y*100:.4}% respectively')


    state2plot = input('Which region would you like to plot? ')
    if state2plot not in regions:
        state2plot = input("Please choose one of these regions: ('QLD', 'NSW', 'VIC', 'TAS', 'SA', 'NEM')")
        if state2plot not in regions:
            print('Sorry, that is not a valid region.')
    else:
        plot_timeseries(date_d, gwh_d, reldiff_d, state2plot)
            
    
    
    
if __name__ == "__main__":
    main()

