import os
import matplotlib.pyplot as plt
import numpy as np
import math

maxsize = 10
T = 2
folder = f'maxsize={maxsize};T={T}'

folder_list = [
    'maxsize=10;T=2',
    'maxsize=10;T=3',
    'maxsize=13;T=2',
]
colors = ['red', 'blue', 'yellow']

for file in os.listdir(folder):
    fig, axs = plt.subplots(2)
    fig.suptitle(file)
    all_get_data = []
    for n,cur_folder in enumerate(folder_list):
        with open(f'{cur_folder}/{file}', 'r+') as fp:
            content = fp.read().split('\n')
        # put plot
        put_data = [
            math.log(int(i[4:])) 
            if int(i[4:]) 
            else int(i[4:])

            for i in content 
            if i.startswith('put ')
        ]
        x = np.asarray(
            [i for i in range(len(put_data))]
        )
        
        axs[0].stem(
            x, 
            put_data, 
            use_line_collection=True,
            markerfmt=f'C{n}o',
            label=cur_folder,
        )
        
        # get plot
        get_data = [
            math.log(int(i[4:])) 
            if int(i[4:]) 
            else int(i[4:])

            for i in content 
            if i.startswith('get ')
        ]
        all_get_data.append(get_data)

    axs[0].set_xlabel('n-th put')
    axs[0].set_ylabel('Put Log Latency (microseconds)')
    axs[0].legend()
    # axs[0].subplots_adjust(
    #     left=0.10, 
    #     bottom=0.11, 
    #     right=0.97, 
    #     top=0.92, 
    #     wspace=0.2, 
    #     hspace=0.28
    # )
    axs[1].boxplot(
        all_get_data,
        vert=False,  
        # vertical box alignment
        patch_artist=True,  
        # fill with color
        labels=folder_list
        # will be used to label x-ticks
    )  
    axs[1].set_xlabel('Get Log Latency (microseconds)')
    axs[1].set_ylabel('Three configs')
    plt.subplots_adjust(
        left=0.24, 
        bottom=0.11, 
        right=0.97, 
        top=0.92, 
        wspace=0.2, 
        hspace=0.28
    )
    # fig.tight_layout()
    plt.savefig(f'{file}.png')
    # plt.show()