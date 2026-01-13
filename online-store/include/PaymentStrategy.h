#pragma once

#include <iostream>
#include <string>

class PaymentStrategy {
public:
	virtual void pay(double amount) = 0;
	virtual ~PaymentStrategy() = default;
};

class CardPayment : public PaymentStrategy {
private:
	std::string cardNumber;
	std::string cardHolder;
public:
	CardPayment(const std::string& cn, const std::string& ch);

	void pay(double amount) override;
};

class WalletPayment : public PaymentStrategy {
private:
	std::string walletId;
public:
	WalletPayment(const std::string& id);

	void pay(double amount) override;
};

class SBPPayment : public PaymentStrategy {
private:
	std::string phoneNumber;
public:
	SBPPayment(const std::string& pn);

	void pay(double amount) override;
};
