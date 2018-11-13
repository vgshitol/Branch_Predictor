//
// Created by Vishal Shitole on 11/12/18.
//

#ifndef BRANCH_PREDICTOR_BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_BRANCH_PREDICTOR_H

#include <vector>
#include <math.h>
#include <iostream>

using namespace std;

typedef enum Predictor_Type {
    BIMODAL,
    GSHARE,
    HYBRID
}predictor_type;

typedef struct Performance_Parameters {
    unsigned long int numberOfPredictions = 0;
    unsigned long int numberOfMisPredictions = 0;
    float rateOfMisPrediction = 0.0;

}performance_parameters;

typedef struct Branch{
    unsigned long int index = 0;
    char actualTNT = 'n';
    char predictedTNT = 'n';
    unsigned short counter = 0b10;
    unsigned long int bhr = 0b00;
}branches;

typedef struct predictionModelParameters{
    unsigned long numberOfIndexes;
    unsigned short indexBitLength;
    unsigned short bhrBitLength;
    unsigned long int bhr;
    unsigned long int bhrMaskValue;
    vector<branches> branch;
    branches currentBranch;
}prediction_model_parameters;

class Branch_Predictor {

public:
    prediction_model_parameters pmp[3];
    predictor_type predictorType;
    performance_parameters performanceParameters;
    char actualTNT;
    unsigned long int pc;

    /**
     * Constructor of Branch Predictor
     * @param bp_name
     * @param m2
     * @param n
     * @param m1
     * @param k
     */
    Branch_Predictor(char* bp_name, unsigned short m2, unsigned short n , unsigned short m1, unsigned short k, predictor_type predictorType) {
        this->predictorType = predictorType;
        switch (predictorType) {
            case BIMODAL:
                initializeBimodal(m2);
                break;
            case GSHARE:
                initializeGShare(m1, n);
                break;
            case HYBRID:
                initializeBimodal(m2);
                initializeGShare(m1, n);
                initializeChooser(k);
                break;
            default:
                break;
        }
    }

    void initializeBimodal(unsigned short m2){
        pmp[BIMODAL].indexBitLength = m2;
        pmp[BIMODAL].numberOfIndexes = (unsigned long) (pow(2, pmp[BIMODAL].indexBitLength));
        pmp[BIMODAL].branch.resize(pmp[BIMODAL].numberOfIndexes);
        cout << "BRANCH SIZE M2: " << pmp[BIMODAL].branch.size() << endl;
        pmp[BIMODAL].bhrBitLength = 0;
        this->pmp[BIMODAL].bhr = 0;
        this->pmp[BIMODAL].bhrMaskValue = 0; //(unsigned long)(pow(2,  pmp[BIMODAL].bhrBitLength)) - 1;
        this->predictorType = BIMODAL;
    }

    void initializeGShare(unsigned short m1, unsigned short n){
        this->predictorType = GSHARE;
        pmp[GSHARE].indexBitLength = m1;
        pmp[GSHARE].numberOfIndexes = (unsigned long) (pow(2, pmp[GSHARE].indexBitLength));
        pmp[GSHARE].branch.resize(pmp[GSHARE].numberOfIndexes);
        cout << "BRANCH SIZE M1: " << pmp[GSHARE].branch.size() << endl;
        pmp[GSHARE].bhrBitLength = n;
        this->pmp[GSHARE].bhr = 0;
        this->pmp[GSHARE].bhrMaskValue = (unsigned long) (pow(2, pmp[GSHARE].bhrBitLength)) - 1;
    }

    void initializeChooser(unsigned short k){
        this->predictorType = HYBRID;
        pmp[HYBRID].indexBitLength = k;
        pmp[HYBRID].numberOfIndexes = (unsigned long) (pow(2, pmp[HYBRID].indexBitLength));
        pmp[HYBRID].branch.resize(pmp[HYBRID].numberOfIndexes);
        cout << "BRANCH SIZE K: " << pmp[HYBRID].branch.size() << endl;
        for (unsigned long int i = 0; i < pmp[HYBRID].branch.size(); i++) pmp[HYBRID].branch.at(i).counter = 1;
    }

    unsigned long int getIndex(){
        unsigned long int index;
        index = (pc  &  ((pmp[predictorType].numberOfIndexes-1) << 2)) >> 2 ;
        index = index^(pmp[predictorType].bhr << (pmp[predictorType].indexBitLength-pmp[predictorType].bhrBitLength));
        (predictorType == BIMODAL) ? cout << "BP:   " << index << "\t" : cout << "GP:   " << index << "\t" ;
        return index;
    }

    bool getPrediction(unsigned long int index,branches * currentBranch){
        *currentBranch = this->pmp[predictorType].branch.at(index);
        currentBranch->index = index;
        currentBranch->actualTNT = actualTNT;

        if(currentBranch->counter >= 2 ) currentBranch->predictedTNT =  't';
        else currentBranch->predictedTNT =  'n';

        return actualTNT == currentBranch->predictedTNT;
    }

    void updateBHR(){
        unsigned long int bhr;
        bhr = pmp[predictorType].bhr & pmp[predictorType].bhrMaskValue;
        if(actualTNT == 't') bhr = ((1 << (pmp[predictorType].bhrBitLength-1)) + (bhr>>1)) & pmp[predictorType].bhrMaskValue;
        else bhr = bhr >> 1;
        pmp[predictorType].bhr = bhr;
        cout << "BHR: \t " <<bhr << endl;
    }

    void updateBranchTable(branches currentBranch){
        this->pmp[predictorType].branch.at(currentBranch.index) = currentBranch;
    }

    bool gSharePrediction(){
        unsigned long int index;
        branches currentBranch;
        bool predictionStatus;

        index = getIndex();
        predictionStatus = getPrediction( index, &currentBranch);
        updateCounter(&currentBranch.counter);
        updateBHR();
        updateBranchTable(currentBranch);

        return predictionStatus;
    }



    bool hybridPrediction(){
        bool chooser; // Gshare , false Bimodal
        unsigned short int predictionStatus = 0;
        // 0 - G0 B0 Fail, 1 - G0 B1 - Bimodal, 2 - G1 B0 - GShare, 3 - G1 B1 Both

        branches currentBranch, gshareBranch, bimodalBranch;
        unsigned long int index, indexGshare, indexBimodal;

        index = getIndex();
        getPrediction( index, &currentBranch);

        this->predictorType = BIMODAL;
        indexBimodal = getIndex();
        predictionStatus |= getPrediction(indexBimodal, &bimodalBranch);

        this->predictorType = GSHARE;
        indexGshare = getIndex();
        predictionStatus |= (getPrediction(indexGshare, &gshareBranch)<< 1);

        this->predictorType = HYBRID;
        chooser = choosePredictor(&currentBranch, index);

// update counter of the taken branch
        if(chooser) {
            this->predictorType = GSHARE;
            updateCounter(&gshareBranch.counter);
            updateBranchTable(gshareBranch);

        }
        else {
            this->predictorType = BIMODAL;
            updateCounter(&bimodalBranch.counter);
            updateBranchTable(bimodalBranch);

        }
        // Update BHR
        this->predictorType = GSHARE;
        updateBHR();

        // update chooser
        this->predictorType = HYBRID;
        updateChooser(predictionStatus, &currentBranch.counter);
        updateBranchTable(currentBranch);

        return static_cast<bool>((chooser) ? ((predictionStatus >> 1) & 1) : (predictionStatus & 1));
    }


    bool choosePredictor(branches * currentBranch, unsigned long int index){
        *currentBranch = this->pmp[HYBRID].branch.at(index);
        currentBranch->index = index;
        return currentBranch->counter >= 2;
    }

    void branchPrediction(unsigned long int pc, char actualTNT){
        unsigned long int indexM1, indexM2;
        performanceParameters.numberOfPredictions++;
        this->pc = pc;
        this->actualTNT = actualTNT;
        switch (this->predictorType){
            case BIMODAL:
                if(!gSharePrediction()) performanceParameters.numberOfMisPredictions++;
                //     if(!bimodalPrediction(pc,actualTNT)) performanceParameters.numberOfMisPredictions++;
                break;
            case GSHARE:
                if(!gSharePrediction()) performanceParameters.numberOfMisPredictions++;
                break;
            case HYBRID:
                if(!hybridPrediction()) performanceParameters.numberOfMisPredictions++;
                break;
            default: break;
        }
    }

    void updateCounter(unsigned short * counter){
        switch (*counter){
            case 0:
                *counter = (actualTNT == 't') ? 1 : 0;
                break;
            case 1:
                *counter = (actualTNT == 't') ? 2 : 0;
                break;
            case 2:
                *counter = (actualTNT == 't') ? 3 : 1;
                break;
            case 3:
                *counter = (actualTNT == 't') ? 3 : 2;
                break;
            default: break;

        }
        cout << *counter << endl;

//        cout << "COUNTER VALUE : " << counter << endl;
    }

    void updateChooser(unsigned short predictionStatus, unsigned short * counter){
        cout << *counter << "\t";
        if((predictionStatus == 2) && (*counter < 3)){
            *counter = static_cast<unsigned short>(*counter + 1);
            cout<< "CU: \t" << *counter << endl;
        }
        else  if((predictionStatus == 1) && (*counter > 0)) {
            *counter = static_cast<unsigned short>(*counter - 1);
            cout<< "CU: \t" << *counter << endl;
        }
    }

    void printBranchK(){
        for (int i = 0; i < pmp[HYBRID].branch.size(); ++i) {
            cout << i << "\t" << pmp[HYBRID].branch.at(i).counter << endl;
        }
    }
};


#endif //BRANCH_PREDICTOR_BRANCH_PREDICTOR_H
