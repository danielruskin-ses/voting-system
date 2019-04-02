/*
 * electionmodel.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef MODEL_ELECTIONMODEL_H_
#define MODEL_ELECTIONMODEL_H_

#include "../election/election.h"
#include "../election/electionserver.h"

class ElectionModel
{
private:
	Election _election;
	ElectionServer _electionServer;
public:
	ElectionModel();
	virtual ~ElectionModel();
};

#endif /* MODEL_ELECTIONMODEL_H_ */
