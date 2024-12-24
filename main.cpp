#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

class Category
{
public:
    string name;
    vector<string> words;

    Category() : name("") {}
    Category(string name) : name(name) {}

    void addWord(const string& word)
    {
        words.push_back(word);
    }

    void displayWords() const
    {
        cout << "Words in category " << name << ":" << endl;
        for (size_t i = 0; i < words.size(); ++i)
        {
            cout << i + 1 << ". " << words[i] << endl;
        }
        cout << endl;
    }
};

void populateVocabulary(vector<Category>& vocabulary)
{
    Category animals("Animals");
    animals.words = {"dog", "cat", "cow", "sheep", "bird", "fish", "horse", "duck", "lion", "tiger"};
    vocabulary.push_back(animals);

    Category actions("Common Actions");
    actions.words = {"eat", "drink", "sleep", "go", "come", "walk", "talk", "see", "read", "write"};
    vocabulary.push_back(actions);

    Category colors("Colors");
    colors.words = {"red", "blue", "green", "yellow", "black", "white", "orange", "pink", "brown", "gray"};
    vocabulary.push_back(colors);
}

void customSleep(int milliseconds)
{
    clock_t start_time = clock();
    while (clock() < start_time + milliseconds * (CLOCKS_PER_SEC / 1000));
}

void typeMessage(const string& message, int delay = 50)
{
    for (const char& ch : message)
    {
        cout << ch;
        cout.flush();
        customSleep(delay);
    }
    cout << endl;
}

void playVoice(const string& word)
{
    cout << "[Voice Playing]: " << word << endl;
}

void displayMenu()
{
    cout << "==========================" << endl;
    cout << "          LEXIMO          " << endl;
    cout << " The English Learning App " << endl;
    cout << "==========================" << endl;
}

int main()
{
    displayMenu();
    customSleep(2000);
    system("cls");

    string fullName;
    cout << "Please enter your full name: ";
    getline(cin, fullName);

    string firstName;
    size_t spaceIndex = fullName.find(' ');
    if (spaceIndex != string::npos)
    {
        firstName = fullName.substr(0, spaceIndex);
    }
    else
    {
        firstName = fullName;
    }

    customSleep(1000);
    system("cls");

    cout << endl;
    cout << "Hi " << firstName << "," << endl;
    typeMessage("We warmly welcome you to Leximo, your companion in learning English!");

    typeMessage("Click anywhere to continue...");
    cin.get();

    customSleep(1000);
    system("cls");

    vector<Category> vocabulary;
    populateVocabulary(vocabulary);

    while (true)
    {
        system("cls");

        cout << "\nAvailable Categories:\n";
        for (size_t i = 0; i < vocabulary.size(); ++i)
        {
            cout << i + 1 << ". " << vocabulary[i].name << endl;
        }

        cout << "\nEnter the category number to explore its words (or type 'exit' to quit): ";
        string input;
        getline(cin, input);

        if (input == "exit")
        {
            typeMessage("Goodbye " + firstName + "! Have a great day ahead!");
            customSleep(1000);
            break;
        }

        if (isdigit(input[0]))
        {
            int categoryChoice = stoi(input) - 1;
            if (categoryChoice >= 0 && categoryChoice < vocabulary.size())
            {
                vocabulary[categoryChoice].displayWords();

                cout << "\nEnter the number of the word to hear its pronunciation: ";
                string wordChoice;
                getline(cin, wordChoice);

                if (isdigit(wordChoice[0]))
                {
                    int wordIndex = stoi(wordChoice) - 1;
                    if (wordIndex >= 0 && wordIndex < vocabulary[categoryChoice].words.size())
                    {
                        string word = vocabulary[categoryChoice].words[wordIndex];
                        playVoice(word);
                    }
                    else
                    {
                        cout << "Invalid word selection. Please try again." << endl;
                        customSleep(1000);
                    }
                }
                else
                {
                    cout << "Invalid input. Please enter a valid number." << endl;
                    customSleep(1000);
                }
            }
            else
            {
                cout << "Invalid category selection. Please try again." << endl;
                customSleep(1000);
            }
        }
        else
        {
            cout << "Invalid input. Please enter a valid category number." << endl;
            customSleep(1000);
        }
    }

    return 0;
}
