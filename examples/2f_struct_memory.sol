pragma solidity ^0.5;

contract Addition {
    struct Bid {
        uint id;
        uint deposit;
    }
    int left = 10;
    int count = 5;

    function add(int a) public pure returns(int result){
        result = a + 4;
    }

    function bid(uint a) public pure returns(uint result){
        Bid storage b = Bid(8,10);
        result=b.id+ a;
        return result;
    }
}
