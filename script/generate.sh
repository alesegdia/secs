#!/usr/bin/env bash

# DESCRIPTION
# 	This script is a quick and dirty way to create new components and systems
# USAGE
# 	./generate.sh [ecs_folder] [command] [item_name] [requirements]
# 		+ [ecs_folder]: the folder where must lie components.h, systems.h and both system and component folders
#		+ [command]: is one of newsystem or newcomponent
# 		+ [item_name]: the name of the system or the component (without system or component)
#		+ [requirements]: a list of components needed by the system (not used when creating new components)
#
# EXAMPLES
#   ./generate.sh src/ecs newcomponent HealthComponent
# 		Generates struct HealthComponent in src/ecs/component/healthcomponent.h
#	./generate.sh src/ecs newsystem DamageSystem HealthComponent DamageComponent
# 		Generates class DamageSystem in src/ecs/system/damagesystem.h, processing every entity
# 		that has both a HealthComponent and a DamageComponent
#
# The name is used verbatim (no suffix is added). The component and system folders must
# already exist. Requirements must name existing components; the generated Process()
# gets one reference parameter per requirement, named after it in lowercase.

FOLDER=$1
shift
CMD=$1
shift
ARG=$1
shift

if [ ! -d "$FOLDER" ]; then
	echo "$FOLDER is not a folder!"
	exit
fi

function build_header_file {
	echo "Rebuilding $1s.h..."
	SUBFOLDER=$1
	HEADER="#pragma once"$'\n'$'\n'
	for element in $FOLDER/$SUBFOLDER/*.h; do
		b=$(basename $element)
		HEADER+="#include \"$SUBFOLDER/$b\""$'\n'
	done
	echo -e "$HEADER" > ${FOLDER}/${SUBFOLDER}s.h
}

function build_prefab {
	echo -e "#pragma once"$'\n'
	echo -e "struct ECSPrefab {"$'\n'
	for element in $FOLDER/component/*.h; do
		grep -oP "struct (.*)Component" $element | while read -r line; do
			COMPONENTNAME=$(echo $line | cut -d " " -f2)
			echo -e "\t$COMPONENTNAME* m_$COMPONENTNAME;"
		done
	done
	echo -e "}"
}

function regenerate_headers {
	build_header_file system
	build_header_file component
}

if [ "$CMD" == "newcomponent" ]; then
	COMPONENT_FILE="${FOLDER}/component/${ARG,,}.h"
	if [ -f $COMPONENT_FILE ]; then
		echo "Error trying to create component! File $COMPONENT_FILE already exists!"
		exit 1
	fi
	
	if [ "$ARG" == "" ]; then
		echo "Error, you must provide the new component name"
		exit
	fi
	
	echo "Creating new component ${ARG}"
	
	COMPONENT_SOURCE="#pragma once\n\nstruct ${ARG}\n{\n\n};"
	echo -e $COMPONENT_SOURCE > ${COMPONENT_FILE}
fi

if [ "$CMD" == "newsystem" ]; then
	SYSTEM_FILE="${FOLDER}/system/${ARG,,}.h"
	if [ -f $SYSTEM_FILE ]; then
		echo "Error trying to create system! File $SYSTEM_FILE already exists!"
		exit
	fi
	
	if [ "$ARG" == "" ]; then
		echo "Error, you must provide the new system name"
		exit
	fi

	HAS_REQUIREMENTS="0"
	REQUIREMENTS_LINE=""
	REQUIREMENTS_PROCESS=""
	CTR=1
	while (( "$#" )); do
		HAS_REQUIREMENTS="1"
		NEEDED_COMPONENT=$1
		shift
		REQUIREMENTS_LINE+="$NEEDED_COMPONENT, "
		REQUIREMENTS_PROCESS+="$NEEDED_COMPONENT& ${NEEDED_COMPONENT,,}, "
		CTR=$((CTR+1))
	done
	
	REQUIREMENTS_PROCESS=${REQUIREMENTS_PROCESS::-2}
	REQUIREMENTS_LINE=${REQUIREMENTS_LINE::-2}
	
	if [ "$HAS_REQUIREMENTS" == "0" ]; then
		echo "You need to specify some component requirements"
		exit
	fi
	
	echo "Generating system at $SYSTEM_FILE..."
	
	SYSTEM_SOURCE="
#pragma once

#include <secs/secs.h>
#include \"../components.h\"

class ${ARG} : public secs::TypedEntitySystem<${REQUIREMENTS_LINE}>
{
public:
	SECS_SYSTEM(${ARG})

	// Called every Engine::Step for each entity that has all the required components.
	void Process(double delta, const secs::Entity& e, ${REQUIREMENTS_PROCESS}) override
	{

	}
};	
"
	echo -e "$SYSTEM_SOURCE" > $SYSTEM_FILE
	
fi

regenerate_headers
# build_prefab