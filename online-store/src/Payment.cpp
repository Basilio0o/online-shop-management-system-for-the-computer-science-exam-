#include "../include/Payment.h"
using namespace std;

CardPayment::CardPayment(const string& cn, const string& ch) : cardNumber(cn), cardHolder(ch) {
    if (cn.length() != 16) {
        throw std::invalid_argument("Некорректный номер карты: слишком короткий.");
    }
    if (ch.empty()) {
        throw std::invalid_argument("Имя держателя карты не может быть пустым.");
    }
}

void CardPayment::pay(double amount) {
	if(amount < 0) {
		throw runtime_error("Платёж отклонён: сумма должна быть больше нуля.");
	}
	cout << "Оплата картой\nСумма: " << amount << " руб.\nКарта: **** **** **** "
			<< cardNumber.substr(cardNumber.size() - 4) << "\nСовершил оплату: " << cardHolder << endl;
}

WalletPayment::WalletPayment(const string& id) : walletId(id) {
    if (walletId.empty()) {
        throw std::invalid_argument("Id кошелька не может быть пустым.");
    }
}

void WalletPayment::pay(double amount) {
	if(amount < 0) {
		throw runtime_error("Платёж отклонён: сумма должна быть больше нуля.");
	}
	cout << "Оплата электронным кошельком\nСумма: " << amount << " руб.\nКошелёк: " << walletId << endl;
}

SBPPayment::SBPPayment(const string& pn) : phoneNumber(pn) {
    if (phoneNumber.empty()) {
        throw std::invalid_argument("Телефон не может быть пустым");
    }
}

void SBPPayment::pay(double amount) {
	if(amount < 0) {
		throw runtime_error("Платёж отклонён: сумма должна быть больше нуля.");
	}
	cout << "Оплата Через СПБ\nСумма: " << amount << " руб.\nТелефон: " << phoneNumber << endl;
}
