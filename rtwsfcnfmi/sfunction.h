#pragma once 

#if defined(_MSC_VER)
#include "windows.h" /* for HINSTANCE */
#endif

#include "simstruc.h"
#include "model_interface.h"

#define SFCN_FMI_MAX_TIME  1e100
#define SFCN_FMI_EPS       2e-13  /* Not supported with discrete sample times smaller than this */


/* Model status */
typedef enum {
	modelInstantiated,
	modelInitializationMode,
	modelEventMode,
	modelContinuousTimeMode,
	modelTerminated
} ModelStatus;

/* forward declare Model type */
typedef struct Model_s Model;

typedef enum {
	OK = 0,
	Warning = 1,
	// Discard = 2,
	Error = 3,
	Fatal = 4,
	// Pending = 5
} Status;

typedef void (*logMessageCallback)(Model *model, Status status, const char *message, ...);

/* Model data structure */
struct Model_s {
	void *userData;
	logMessageCallback logMessage;
	const char* instanceName;
	int loggingOn;
	SimStruct* S;
	real_T* dX;
	real_T* oldZC;
	int_T* numSampleHits;
	int_T fixed_in_minor_step_offset_tid;
	real_T nextHit_tid0;
	real_T lastGetTime;
	int shouldRecompute;
	int isCoSim;
	int isDiscrete;
	int hasEnteredContMode;
	real_T time;
	real_T nbrSolverSteps;
	ModelStatus status;
#if defined(_MSC_VER)
	HINSTANCE* mexHandles;
#else
	void** mexHandles;
#endif
	real_T* inputDerivatives;
	real_T derivativeTime;
    ModelVariable modelVariables[N_MODEL_VARIABLES];
};

/* Function to copy per-task sample hits */
void copyPerTaskSampleHits(SimStruct* S);

Model *InstantiateModel(const char* instanceName, logMessageCallback logMessage, void *userData);

SimStruct *CreateSimStructForFMI(const char* instanceName);

typedef void(*FreeMemoryCallback)(void*);

void FreeSimStruct(SimStruct *S);
void FreeModel(Model* model);
void resetSimStructVectors(SimStruct *S);
void ReseModel(Model* model);
void allocateSimStructVectors(Model* m);
void setSampleStartValues(Model* m);
void NewDiscreteStates(Model *model, int *valuesOfContinuousStatesChanged, real_T *nextT);

/* ODE solver functions */
extern void rt_CreateIntegrationData(SimStruct *S);
extern void rt_DestroyIntegrationData(SimStruct *S);
extern void rt_UpdateContinuousStates(SimStruct *S);
extern void rt_InitInfAndNaN(size_t realSize);
