# TODOs - Version 3: Distributed Rendering
* Create a distributed rendering service using the distributed command pattern
    * Command process/computer
    * Controlled processes/computers
    * TCP for transmitting commands/data
    * Available commands
        * DOWNLOAD - Sends data to a computer
        * RENDER   - Renders a frame on a computer
    * Available responses
        * OK       - Acknowledges a command

* Convert geometry to tangent space to reduce math operations

* Include curved objects (cylinders, conics, Bezier curves)

# TODOs - Always needed
* Find a better file format or parse other formats into this format
    * Configured a custom format dubbed SDML (Scene Description Markup Language); Still needs a lot of work but basic interface is implemented

* Implement other materials (plastic, matte surfaces, metals, ...) and update Material interface - Copper, Gold, Plastic added

* Add more benchmarking scenes

* Document everything

* Create a testing suite
