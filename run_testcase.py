import os

path = "C:\Research\ThOP\ThOP_solver\experiments_verysmalldata"
# print(os.path.isdir(path))
list_input = os.listdir(path)
random_seeds = [ 269070,  99470, 126489, 644764, 547617, 642580,  73456, 462018, 858990, 756112]
#Phero.exe <inputfile> <RUN_TIME> <rngSeed>

with open('thongke.csv', 'w') as fileOut:
    for name_input_file in list_input:
        out = name_input_file
        os.system("main.exe experiments_verysmalldata\%s 23521285 5" % (name_input_file))
        with open('main.out', 'r') as fileInp:
            result = fileInp.readline()
            if (len(result) == 0): result = "######"
            # else: result = result - "\n"
            else: result = int(result)
            out = out + ',' + str(result)
        out = out + '\n'
        fileOut.write(out)
        print("Epoch:",name_input_file," done\n")