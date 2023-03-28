import scipy.io.wavfile
from scipy.io.wavfile import read
from scipy.io.wavfile import write
import scipy.signal as signal
import numpy as np
import matplotlib.pyplot as plt

# Lucas Schuurmans-Stekhoven
# Student No. 46363468

FILTER_FUNCTION_LENGTH = 61

def read_wav_file(input_filename):
    """ Takes a .wav filename as an input parameter and returns the sampling rate.

    (i.e. the number of signal samples per second) and a tuple containing all the
    signal samples. The function only reads mono (i.e. single channel .wav files).

    Parameters:
        input_filename (str): The name of the input .wav file.

    Returns:
        tuple(int, tuple(numpy.int16)): The first integer of the tuple represents the
        sampling rate, the second element of the tuple is another tuple containing the
        samples of the input signal. 
    """

    sr_value, x_value = read(input_filename)
    return sr_value, tuple(x_value)




def create_filter_function(cutoff, filter_type):
    """ Returns a weighting function for a given cutoff and filter type.

    Parameters:
        cutoff (float): The cutoff frequency (kHz) (between 0.001 and 22.05).
        filter_type (str): The filter type (‘lowpass’ or ‘highpass’).

    Returns:
        (tuple): The samples of the weighting_function.
    """
   
    if filter_type == 'lowpass':
        switch_value=True
    else:
        switch_value=False
    filter_function=scipy.signal.firwin(FILTER_FUNCTION_LENGTH,
    cutoff/22.05,window='hamming',pass_zero=switch_value)
    return tuple(filter_function)

    


def write_wav_file(output_file_name, sr_value, signal):
    """ Outputs a mono (single channel) signal to a .wav file.

    Parameters:
        output_filename (str): The name of the output .wav file.
        sr_value (int): The sampling rate.
        signal(tuple): A tuple of samples of the output signal.

    Returns:
        None.
    """

    signal=np.array(signal).astype(np.int16)
    write(output_file_name, sr_value, signal)

    


def compute_filtered_signal(in_signal, weighting_function):
    """ Creates a filtered signal.

    The function takes in data from in_signal and weighting_function and
    the data is then input into a mathematical signal filtering function
    which returns a tuple containing the filtered signal data.

    Parameteres:
        in_signal(tuple): Sample data from the input signal (wav file).
        weighting_function (tuple): Weighting data based on specified
        cutoff frequency and filter type.

    Returns:
        filtered_output(tuple): Data for the filtered signal.
    """    

    filtered_output = ()

    for sample in range(60, len(in_signal)): 
        filtered_sample_output = 0
        # The below loop for 'length' executes the filtering function summation.
        # The filtering summation is done for each 'sample' in the above loop.
        for length in range(FILTER_FUNCTION_LENGTH): 
            filtered_sample_output += weighting_function[length]*in_signal[sample - length] 
                        
        filtered_sample_output = round(filtered_sample_output, 2)
        filtered_output += (filtered_sample_output,)
        
    return tuple(filtered_output)




def filter_signal(in_signal, cutoff, filter_type):
    """ Calls two functions to create a filtered signal.    

    Parameters:
        in_signal(tuple): Sample data from the input signal (wav file).
        cutoff(int): Specified cutoff frequency.
        filter_type(str): Chosen filter (either lowpass or highpass).

    Returns:
        filtered_signal(tuple): Data for the filtered signal.
    """
    weighting_function = create_filter_function(cutoff, filter_type)
    filtered_signal = compute_filtered_signal(in_signal, weighting_function)
    
    return tuple(filtered_signal)




def mix_signals(signal_1, weight_1, signal_2, weight_2):
    """ Mixes two signals together each with their own weighting.

    Takes in a tuple of values from signal 1 and signal 2 which are then
    multiplied by seperate weightings and added together to form a combined
    signal which returns the tuple of that combined signal.

    Parameters:
        signal_1(tuple): tuple of values from an input_signal.
        weight_1(float): weighting value between 0 and 1.
        signal_2(tuple): tuple of values from an additive signal.
        weight_2(float): weighting value between 0 and 1.

    Returns:
        tuple(mixed_signal): tuple of two combined signals.
    """
    
    mixed_signal = ()    
    
    for sample in range(len(signal_1)):
        mix = (signal_1[sample]*weight_1 + signal_2[sample]*weight_2)
        mix = round(mix,2)
        mixed_signal += (mix,)

    return tuple(mixed_signal)




def file_names():
    """ Function for the repeated request to enter input and output file names.

    Parameters:
        input_filename(str): The name of the input file.
        output_file_name(str): The name of the output file.

    Returns:
        input_filename, output_file_name
    """
    
    input_filename = str(input('Please enter the input filename: '))
    output_file_name = str(input('Please enter the output filename: '))

    return input_filename, output_file_name




def weighting_specs():
    """ Requests weighting specifications with incorrect input functionality.

    If initial input of filter_type is incorrect it moves to a while loop
    which will repeatedly request for a correct filter_type input.

    Parameters:
        cutoff(float): Cutoff frequency.
        filter_type(str): Filter type (lowpass or highpass).

    Returns:
        filter_type, cutoff
    """

    filter_type = (input('Please specify filter type - lowpass or highpass: '))

    if filter_type == 'lowpass' or filter_type == 'highpass':
        cutoff = float(input('Please enter the cutoff frequency (kHz) [0.001 - 22.05]: '))
    else:
        filter_type = ''
        while(not(filter_type == 'lowpass' or filter_type == 'highpass')):
                print('That is not a valid filter type.')
                filter_type = (input('Specify filter type - lowpass or highpass: '))
        cutoff = float(input('Please enter the cutoff frequency (kHz) [0.001 - 22.05]: '))
                  
    return filter_type, cutoff
             
    
   

def main():
    """ This program is designed to modify single channel wav files.

    Functionality includes changing playback speed, mixing two signals
    together, filtering a signal or quitting. Each option will request
    user input and then write the modified audio file to the specified PATH.
    Once complete it will continue to request a selection until user quits.
    """
    
    while True:
        selection = input('Please select one of the following four options:\n'
                              '    c change the playback speed\n'
                              '    m mix two signals together\n'
                              '    f filter the signal or\n'
                              '    q quit\n'
                              '    : ')
        if selection == 'c':
            input_filename, output_file_name = file_names()
            samp_rate, signal = read_wav_file(input_filename)
            speed_adjustment = float(input('Please enter the speed adjustment factor (0.5-2.0): '))
            adjusted_rate = samp_rate*speed_adjustment
            adjusted_rate = int(adjusted_rate)
            write_wav_file(output_file_name, adjusted_rate, signal)
            
        elif selection == 'm':
            input_filename, output_file_name = file_names()   
            additive_filename = (input('Please enter the additive file name: '))    
            weight_1 = float(input('Please enter the weight factor for the input file (0-1.0)'))
            weight_2 = float(input('Please enter the weight factor for the additive file: '))
            samp_rate, signal = read_wav_file(input_filename)
            samp_rate, additive_signal = read_wav_file(additive_filename)        
            mixed_signals = mix_signals(signal, weight_1, additive_signal, weight_2)
            write_wav_file(output_file_name, samp_rate, mixed_signals)             

        elif selection == 'f':
            input_filename, output_file_name = file_names()             
            filter_type, cutoff = weighting_specs()
            samp_rate, signal = read_wav_file(input_filename)       
            filtered_signal = filter_signal(signal, cutoff, filter_type)
            write_wav_file(output_file_name, samp_rate, filtered_signal)

        else:
            break
                   



if __name__ == "__main__":
    main()





    

    
