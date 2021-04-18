#pragma once
#include <math.h>
#include <iostream>
#include <fstream>

namespace Tmpl8 {

	class Utils {
    public:
        // Function that Calculate Root Mean Square
        static float rmsValue(std::vector<int> estimations, std::vector<int> ground_truth)
        {
            float rmse = 0;

            if (estimations.size() != ground_truth.size()
                || estimations.size() == 0) {
                cout << "Invalid estimation or ground_truth data" << endl;

                return rmse;
            }

            for (unsigned int i = 0; i < estimations.size(); ++i) {

                int residual = estimations[i] - ground_truth[i];

                //coefficient-wise multiplication
                residual = residual * residual;

                rmse += residual;
            }

            //calculate the mean
            rmse = rmse / estimations.size();

            //calculate the squared root
            rmse = sqrt(rmse);

            //return the result
            return rmse;
        }

        static void SaveToFile(const std::string& filePath, std::vector<int> values)
        {
            ofstream myfile;

            myfile.open(filePath);
            
            for (auto value : values)
            {
                myfile << value << "\n";
            }

            myfile.close();
        }

        static std::vector<int> LoadFromFile(const std::string& filePath)
        {
            std::vector<int> savedValues;

            string line;
            ifstream myfile(filePath);

            if (myfile.is_open())
            {
                int value = 0;

                while (myfile >> value) {
                    savedValues.push_back(value);
                }

                myfile.close();
            }

            else 
            {
                cout << "Unable to open file";
            }

            return savedValues;
        }


    private:
        Utils() {}
	};
}