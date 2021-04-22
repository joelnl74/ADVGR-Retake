#pragma once
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace Tmpl8 {

    static std::vector<int> m_GroundTruth;
    static std::random_device rd;
    static std::default_random_engine eng(rd());

    std::uniform_real_distribution<float> distr(0.0f, 1.0f);

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

        static std::vector<int> GetGroundTruth()
        {
            if (m_GroundTruth.size() == 0)
            {
                m_GroundTruth = LoadFromFile("GroundTruth.txt");
            }

            return m_GroundTruth;
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

        static void SaveToFile(const std::string& filePath, std::vector<float> values)
        {
            ofstream myfile;

            myfile.open(filePath);

            for (auto value : values)
            {
                myfile << value << "\n";
            }

            myfile.close();
        }


        static float RoundFloat(float var)
        {
            float value = (int)(var * 100 + .5);
         
            return (float)value / 100;
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

        static float RandomNumber()
        {       
            float value = distr(eng);

            return value;
        }


    private:
        Utils() {}
	};
}