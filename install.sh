#!/bin/bash

## Script d'installation temporaire

echo "Début de l'installation"
export LD_LIBRARY_PATH=./libs/libpng12
echo $LD_LIBRARY_PATH
make 
echo "Installation terminée"

bash -i