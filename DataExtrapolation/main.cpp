#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Query {
    string brand;
    string category;
    double price;
    bool isOnSale;
    bool delivery;
};

struct Product {
    string brand = "";
    string category = "";
    double price = 0;
    string orderedOnline = "";
    string paidFullPrice = "";
    int yearOfPurchase = 0;
};

vector<Product> readCSV(const string& filename) {
    vector<Product> products;
    ifstream file(filename);
    string line;

    // Check if the file was successfully opened
    if (!file.is_open()) {
        cout << "Failed to open file: " << filename << endl;
        return products;
    }

    // Read the header line
    getline(file, line);

    // Read the data lines
    while (std::getline(file, line)) {
        istringstream iss(line);
        string value;

        // Create a new Product instance
        Product product;

        // Read each value separated by commas
        getline(iss, value, ',');
        product.brand = value;

        getline(iss, value, ',');
        product.category = value;

        getline(iss, value, ',');
        product.price = stod(value);

        getline(iss, value, ',');
        product.orderedOnline = product.orderedOnline == "Yes" ? "true" : "false";

        getline(iss, value, ',');
        product.paidFullPrice = product.paidFullPrice == "Yes" ? "true" : "false";

        getline(iss, value, ',');
        product.yearOfPurchase = stoi(value);

        // Add the product to the vector
        products.push_back(product);
    }

    // Close the file
    file.close();

    return products;
}

bool shouldBeExcluded(Product product, Query query) {
    int currentYear = 2023;
    int yearGap = 3; // change this to your liking (bigger yearGap means the algorithm will take older purchases as valid entries)
    int priceGap = 25; // this represents the percentage (more) of the price gap between user's query and products

    // exclusions start from the price, since it is more important than other product attributes

    if ((product.price - query.price) / query.price * 100 > priceGap) {
        return true;
    }

    // If the product is not purchased in the last 3 years, it should be excluded
    if (currentYear - product.yearOfPurchase > 3) {
        return true;
    }

    return false;
}

vector<Product> inclusiveEntries(vector<Product> purchaseHistory, Query query) {
    vector<Product> exclusion;
    bool brandExists = false;
    bool categoryExists = false;

    for (int i = 0;i < purchaseHistory.size(); i++) {
        if (purchaseHistory[i].brand == query.brand) {
            brandExists = true;
        }
        if (purchaseHistory[i].category == query.category) {
            categoryExists = true;
        }
    }

    if (brandExists && categoryExists) {
        for (int i = 0; i < purchaseHistory.size(); i++) {
            if (!shouldBeExcluded(purchaseHistory[i], query)) {
                exclusion.push_back(purchaseHistory[i]);
            }
        }
    }

    return exclusion;
}

double evaluatePossibilities(vector<Product> cleanData, Query query) {
    int currentYear = 2023;
    int toRatio = cleanData.size() + 1;
    int brandPoints = 0, 
        categoryPoints = 0,
        pricePoints = 0, 
        yearPoints = 0;
    bool likesToBuyOnSale = false; 
    bool likesToBuyOnline = false;
    int onlinePurchases = 0; // the number of online purchases made
    int onSalePurchases = 0; // the number of on-sale purchases made

    for (int i = 0; i < cleanData.size(); i++) {
        if (cleanData[i].orderedOnline == "Yes") {
            onlinePurchases++;
        }
        if (cleanData[i].paidFullPrice == "No") {
            onSalePurchases++;
        }
    }

    if (onlinePurchases > cleanData.size() / 2) {
        likesToBuyOnline = true;
    }

    if (onSalePurchases > cleanData.size() / 2) {
        likesToBuyOnSale = true;
    }

    for (int i = 0; i < cleanData.size(); i++) {
        if (cleanData[i].brand == query.brand) {
            brandPoints++;
        }
        if (cleanData[i].category == query.category) {
            categoryPoints++;
        }
        if (cleanData[i].yearOfPurchase >= currentYear-1) {
            yearPoints++;
        }
        if (cleanData[i].price >= query.price) {
            pricePoints++;
        }
    }

    // take the average percentages from points
    double result = ((static_cast<double>(brandPoints) / toRatio) * 100)
        + ((static_cast<double>(categoryPoints) / toRatio) * 100)
        + ((static_cast<double>(yearPoints) / toRatio) * 100)
        + ((static_cast<double>(pricePoints) / toRatio) * 100);
    result = result / 4;

    // In these cases, we're adding a 10% purchase likeliness
    // 1. if the users have made more online purchases 
    if (likesToBuyOnline && query.delivery) {
        result = result + 10;
    }
    // 2. if the users have made more in-store purchases
    if (!likesToBuyOnline && !query.delivery) {
        result = result + 10;
    }
    // 3. if the users like to buy products when on sale
    if (likesToBuyOnSale && query.isOnSale) {
        result = result + 10;
    }

    return result;
}

int main() {
    // Dataset generated by ChatGPT, entries can be changed in a single request.
    vector<Product> purchaseHistory = readCSV("dataset.csv");
    Query query;
    char choice;

    while (true) {
        start:
        system("cls");
        cout << "This query will help you calculate how likely a product will be purchased (respectively: by specified dataset)."
            << "\n\nPlease enter query columns in order: \n"
            << "\nBrand, Category, Price, On sale? (1/0), Can be delivered? (1/0)\n";
        
        cin >> query.brand >> query.category >> query.price >> query.isOnSale >> query.delivery;
        
        vector<Product> cleanData = inclusiveEntries(purchaseHistory, query);
        double result = cleanData.size() > 0 ? evaluatePossibilities(cleanData, query) : 0;
        
        cout << "According to the given dataset, this product has " << result << "% chances to turn profit."
            << "\nContinue ? (y / n)";
        cin >> choice;

        if (choice == 'Y' || choice == 'y') {
            goto start;
        }
    }

    return 0;
}

