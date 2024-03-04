PROJECT_NAME := espri

# include $(IDF_PATH)/make/project.mk

docker:
	./compile-with-docker.sh

flash:
	./flash-with-esptool.sh