pragma solidity ^0.5;

contract Addition {
    //address payable public beneficiary;
    uint highestBid = 20;

    function add() public{
        address payable x = address(0x123);
        address myAddress = address(this);
        if (x.balance < 10 && myAddress.balance >= 10){
            x.transfer(10);
        }
    }

}
