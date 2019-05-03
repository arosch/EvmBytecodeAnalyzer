pragma solidity ^0.5;

contract Addition {
    address payable public beneficiary;

    function add() public{
        beneficiary.transfer(20);
    }
}
