pragma solidity ^0.5;

contract Addition {
    struct Bid {
        uint id;
        uint deposit;
    }
    int left = 10;
    int count = 5;
    
    Bid[] someBids;

    function add(int a) public pure returns(int result){
        result = a + 4;
    }

    function bid() public {
        Bid memory b = Bid(8,10);
        someBids.push(b);
    }
}
