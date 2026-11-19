#include <iostream>
using namespace std;

int main()
{
    int numbers[5] = {1, 2, 3, 4, 5};
    int size = 5;

    int choice;
    int searchkey;
    int position;
    bool found;

    while (true)
    {
        cout << "1. print all elements";
        cout << "2. search for an element";
        cout << "3. calculate index spacing metrics";
        cout << "4. exit";
        cout << "enter your choice (1-4): ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            cout << "Elements in the array: ";
            for (int i = 0; i < size; i++)
            {
                cout << numbers[i] << " ";
            }
            cout << endl;
            break;
        case 2:
            cout << "Enter the number to search for: ";
            cin >> searchkey;

            found = false;
            for (int i = 0; i < size; i++)
            {
                if (numbers[i] == searchkey)
                {
                    position = i;
                    found = true;
                    break;
                }
            }

            if (found)
            {
                cout << "Number " << searchkey << " found at position " << position << "." << endl;
            }
            else
            {
                cout << "Number " << searchkey << " not found." << endl;
            }
            break;
        case 3:
            // Implementation for calculating index spacing metrics
            break;
        case 4:
            cout << "Exiting..." << endl;
            break;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }

        if (choice == 4)
        {
            break;
        }
    }
}    