import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# Use the tkagg backend for interactive plotting
import matplotlib
matplotlib.use('TkAgg')

# Read the CSV file without a header, and manually assign column names
file_path = 'build/out.csv'
column_names = ['time[µs]', 'mA', 'Newton', 'position']

try:
    df = pd.read_csv(file_path, delimiter=';', header=None, names=column_names)
    print("CSV file read successfully.")
except Exception as e:
    print(f"Error reading the CSV file: {e}")

# Check the columns in the DataFrame
print("Columns in DataFrame:", df.columns)

# Create a figure and axis for the first y-axis
fig, ax1 = plt.subplots(figsize=(10, 6))

# Plot the mA data on the first y-axis
ax1.plot(df['time[µs]'], df['mA'], label='mA', color='b')
ax1.set_xlabel('Time [µs]')
ax1.set_ylabel('mA', color='b')
ax1.tick_params(axis='y', labelcolor='b')

# Set number of x-ticks and y-ticks
ax1.xaxis.set_major_locator(ticker.MaxNLocator(nbins=10))
ax1.yaxis.set_major_locator(ticker.MaxNLocator(nbins=10))

# Create a second y-axis for Newton
ax2 = ax1.twinx()
ax2.plot(df['time[µs]'], df['Newton'], label='Newton', color='r')
ax2.set_ylabel('Newton', color='r')
ax2.tick_params(axis='y', labelcolor='r')

# Set number of y-ticks for the second y-axis
ax2.yaxis.set_major_locator(ticker.MaxNLocator(nbins=10))

# Create a third y-axis for position
ax3 = ax1.twinx()
ax3.spines['right'].set_position(('outward', 60))  # Offset the third y-axis
ax3.plot(df['time[µs]'], df['position'], label='position', color='g')
ax3.set_ylabel('Position', color='g')
ax3.tick_params(axis='y', labelcolor='g')

# Set number of y-ticks for the third y-axis
ax3.yaxis.set_major_locator(ticker.MaxNLocator(nbins=10))

# Add a legend
lines, labels = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
lines3, labels3 = ax3.get_legend_handles_labels()
ax1.legend(lines + lines2 + lines3, labels + labels2 + labels3, loc='upper left')

# Add title
plt.title('Data Visualization')

# Show the plot
plt.grid(True)
plt.tight_layout()
plt.show()
