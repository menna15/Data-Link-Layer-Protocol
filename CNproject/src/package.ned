package cnproject;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=503,265");
    submodules:
        coordinator: Coordinator {
            @display("p=236,211;i=block/cogwheel");
        }
        node0: Node {
            @display("p=129,104;i=device/hub");
        }
        node1: Node {
            @display("p=395,118;i=device/hub");
        }
    connections:
        coordinator.out0 --> node0.incor;
        coordinator.out1 --> node1.incor;
        node1.to_pair --> node0.from_pair;
        node0.to_pair --> node1.from_pair;
}

