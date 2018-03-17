# Procedural-Cities

This project aims to provide a framework and a solid implementation of different techniques for generating complete seamless procedural cities with interiors for all buildings.

Small demo: https://www.youtube.com/watch?v=n1eZOV8r_g4

This project was done as a master thesis at LTH, link to thesis is available here: http://lup.lub.lu.se/luur/download?func=downloadFile&recordOId=8929185&fileOId=8929189

The source code is available to everyone under the MIT licence and you're very welcome to clone or fork the reposity, but keep in mind that it is mostly a proof-of-concept and the code is hardly optimized or refined.

Most parameters are accessible from the "Spawner" blueprint-class, one is spawned in the default map, so edit the values inside of that. Some parameters can be found in ProcMeshActorBP, PlotBuilderBP and HouseBuilderBP. The most interesting parameters to change might be length, maximum turn rate for roads and heatmap settings.

Starting the project generates the city, the defalt character can either walk (with collisions enabled) or fly (with collisions disabled), switch by pressing R. 

Some pictures:

![Interior apartment](images/2.png?raw=true "Interior apartment")
![Interior several apartments 1](images/6.png?raw=true "Interior several apartments 1")
![Interior several apartments 2](images/8.png?raw=true "Interior several apartments 2")
![Interior several apartments 3](images/9.png?raw=true "Interior several apartments 3")
![City overview 1](images/city1.PNG?raw=true "City overview 1")
![City overview 2](images/citySS3.PNG?raw=true "City overview 2")
![City overview 3](images/prettypicture.PNG?raw=true "City overview 3")
![City map 1](images/heatmap8_20_5.PNG?raw=true "City map 1")
![City map 2](images/heatmap8_30_15.PNG?raw=true "City map 2")
![City map 3](images/procedural_chaotic.PNG?raw=true "City map 3")
![City map 4](images/procedural_grid.PNG?raw=true "City map 4")
![Large city](images/largecity.PNG?raw=true "Large city")
