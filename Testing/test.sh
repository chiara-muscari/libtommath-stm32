#!/bin/bash

#Variable definition
OPERATION='reduction'
RED_VERSION='classic'
NUM_BITS=32
ASM_OPT='False'
LOG_BIT_NUM=8

#cd Example_test_vectors/ && python3 test_generator.py % 2 no_pattern ${LOG_BIT_NUM}
#cd ..

OLD_TEXT_MAIN="//#include \"../libtommath/Testing/Example_test_vectors/${OPERATION}/test_vec_32.h\""
NEW_TEXT_MAIN="#include \"../libtommath/Testing/Example_test_vectors/${OPERATION}/test_vec_${NUM_BITS}.h\""

#echo -n "Please enter the name of the demo project: "
#read name_p
#PROJECT_NAME=$name_p
PROJECT_NAME="barrett_reduction"

STM32_WORKSPACE="/home/chiara/Documenti/STM32"
LIBTOMATH_PATH="${STM32_WORKSPACE}/${PROJECT_NAME}/Core/libtommath"
STM32CUBEIDE_PATH="/opt/st/stm32cubeide_1.4.0"
CUBE_PROGRAMMER_PATH=${STM32CUBEIDE_PATH}/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_1.4.0.202007081208/tools/bin/STM32_Programmer_CLI 

#Copy the project dir
mv ${STM32_WORKSPACE}/${PROJECT_NAME} ${STM32_WORKSPACE}/${PROJECT_NAME}.bak
mkdir ${STM32_WORKSPACE}/${PROJECT_NAME}
cp -r ${STM32_WORKSPACE}/${PROJECT_NAME}.bak/. ${STM32_WORKSPACE}/${PROJECT_NAME}/

#Replace the main.c with the testing main
cp main ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/
rm ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/main.c
mv ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/main ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/main.c

#Use sed to modify main.c and other headers, according to the test you want to do
sed -i "s|${OLD_TEXT_MAIN}|${NEW_TEXT_MAIN}|g" ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/main.c

sed -i "s|//#define TEST_${OPERATION^^}|#define TEST_${OPERATION^^}|g" ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/main.c

if [ ${NUM_BITS} = 28 ]; then
	sed -i "s|#define STM32|//#define STM32|g" ${LIBTOMATH_PATH}/tommath.h
	sed -i "s|#define MP_32BIT|//#define MP_32BIT|g" ${LIBTOMATH_PATH}/tommath.h
else
	if [ ${ASM_OPT} = 'False' ]; then
		sed -i "s|#define STM32|//#define STM32|g" ${LIBTOMATH_PATH}/tommath.h
	fi

	if [ ${RED_VERSION} = 'folded' ]; then
		sed -i "s|enum reduction_type Reduction_type = standard;|enum reduction_type Reduction_type = folded;|g" ${STM32_WORKSPACE}/${PROJECT_NAME}/Core/Src/main.c
	fi 
fi

#Use the headless build script to build	
${STM32CUBEIDE_PATH}/headless-build.sh -build ${PROJECT_NAME}/Release -data ${STM32_WORKSPACE} -importAll ${STM32_WORKSPACE}/${PROJECT_NAME}/

#program the stm32 and start
${CUBE_PROGRAMMER_PATH} -c port=SWD -d ${STM32_WORKSPACE}/${PROJECT_NAME}/Release/${PROJECT_NAME}.elf  0x08000000 -s

#Delete and restore orig project
rm -rf ${STM32_WORKSPACE}/${PROJECT_NAME}
mv ${STM32_WORKSPACE}/${PROJECT_NAME}.bak ${STM32_WORKSPACE}/${PROJECT_NAME}

#Open a screen session to redirect output
#Note just one serial device, the STM32, should be connected to the PC
screen -L -Logfile test_log /dev/`ls /dev | grep -E 'ttyUSB|ttyACM'` 115200 
echo -n "Finish!"