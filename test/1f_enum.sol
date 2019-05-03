pragma solidity ^0.5;

contract Enum {

    enum Action { Left, Right, Straight, Still }
    Action choice;

    function setStill() public {
        choice = Action.Still;
    }

}
