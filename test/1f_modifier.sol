pragma solidity >0.4.23 <0.6.0;

contract Ex {

    uint public revealEnd;
    int c;

    modifier onlyBefore(uint _time) { require(now < _time); _; }

    function reveal(int a) public onlyBefore(revealEnd){
        c = a+5;
    }

}
