@echo off
g++ main.cpp src/Game.cpp src/EventManager.cpp src/EntityManager.cpp src/AssetManager.cpp src/PhysicsSystem.cpp src/CollisionSystem.cpp src/Log.cpp src/ArchetypeManager.cpp -o build/iguana.exe -O1 -Wall -std=c++23 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
