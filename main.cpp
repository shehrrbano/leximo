#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

// Forward declarations
class Word;
class Category;
class Question;
class Story;

// Structure for a word and its properties
class Word {
public:
    string word;
    string meaning;
    bool isLearned;

    Word(string w, string m) : word(w), meaning(m), isLearned(false) {}
};

// Structure for a category of words
class Category {
public:
    string name;
    vector<Word> words;
    bool isUnlocked;

    Category(string n) : name(n), isUnlocked(true) {}

    void addWord(string word, string meaning) {
        words.push_back(Word(word, meaning));
    }

    void displayWords() {
        cout << "\n=== " << name << " ===\n";
        for (int i = 0; i < words.size(); i++) {
            cout << i + 1 << ". " << words[i].word << endl;
        }
    }
};

// Structure for a story with comprehension questions
class Story {
public:
    string title;
    string content;
    vector<Question*> questions;

    Story(string t, string c) : title(t), content(c) {}

    void display() const {
        cout << "\n=== " << title << " ===\n\n";
        cout << content << endl;
    }
};

// Structure for multiple choice questions
class Question {
public:
    string question;
    vector<string> options;
    int correctAnswer;
    bool isAnswered;
    bool isCorrect;

    Question(string q, vector<string> opts, int correct)
        : question(q), options(opts), correctAnswer(correct),
          isAnswered(false), isCorrect(false) {}

    void display(int num) {
        cout << "\nQuestion " << num << ": " << question << endl;
        for (int i = 0; i < options.size(); i++) {
            cout << char('A' + i) << ") " << options[i] << endl;
        }
    }

    bool checkAnswer(char answer) {
        isAnswered = true;
        int userAnswer = toupper(answer) - 'A';
        isCorrect = (userAnswer == correctAnswer);
        return isCorrect;
    }

    void showCorrectAnswer() {
        cout << "The correct answer is: " << char('A' + correctAnswer)
             << ") " << options[correctAnswer] << endl;
    }
};

// Main application class
class LanguageLearningApp {
private:
    vector<Category> categories;
    vector<Story> stories;
    vector<Question*> allQuestions;
    queue<Question*> mistakeQueue;
    string userName;
    int score;
    int totalWords;
    int wordsLearned;

    void initializeCategories() {
        // Animals Category
        Category animals("Animals");
animals.addWord("Hippopotamus", "Large semiaquatic mammal found in Africa (pronounced: hip-uh-pot-uh-muhs)");
animals.addWord("Rhinoceros", "Large, herbivorous mammal with a horn (pronounced: rye-noss-er-us)");
animals.addWord("Cheetah", "Fastest land animal (pronounced: chee-tuh)");
animals.addWord("Giraffe", "Tallest animal (pronounced: juh-raf)");
animals.addWord("Penguin", "Flightless bird from Antarctica (pronounced: peng-gwin)");
animals.addWord("Zebra", "Striped herbivorous animal from Africa (pronounced: zee-bruh or zeh-bruh)");
animals.addWord("Octopus", "Marine creature with eight tentacles (pronounced: ok-tuh-pus)");
animals.addWord("Platypus", "Egg-laying mammal from Australia (pronounced: plat-uh-pus)");
categories.push_back(animals);

        // Professions Category
Category professions("Professions");

// Medical professions
professions.addWord("Doctor", "Medical professional who treats patients");
professions.addWord("Nurse", "Healthcare professional who cares for patients");
professions.addWord("Surgeon", "Doctor who performs operations");
professions.addWord("Pediatrician", "Doctor specializing in child healthcare");
professions.addWord("Dentist", "Doctor specializing in oral health");
professions.addWord("Pharmacist", "Professional who prepares and dispenses medicines");

// Tech professions
professions.addWord("Programmer", "Person who writes computer code");
professions.addWord("Engineer", "Professional who designs and builds things");
professions.addWord("Analyst", "Person who analyzes data and systems");
professions.addWord("Designer", "Person who creates visual and user experiences");
professions.addWord("Developer", "Person who builds software applications");

categories.push_back(professions);

Category fruits("Fruits");
fruits.addWord("Apple", "Round fruit that can be red or green");
fruits.addWord("Banana", "Long yellow fruit");
fruits.addWord("Orange", "Round citrus fruit");
fruits.addWord("Grape", "Small, sweet fruit growing in clusters");
fruits.addWord("Mango", "Sweet tropical fruit");

Category vegetables("Vegetables");
vegetables.addWord("Carrot", "Orange root vegetable");
vegetables.addWord("Potato", "Starchy root vegetable");
vegetables.addWord("Tomato", "Red fruit used as a vegetable");
vegetables.addWord("Lettuce", "Leafy green vegetable");
vegetables.addWord("Cucumber", "Long green vegetable");

categories.push_back(fruits);
categories.push_back(vegetables);

Category technology("Technology");

// Devices category
technology.addWord("Smartphone", "Mobile phone with advanced computing capability");
technology.addWord("Laptop", "Portable computer");
technology.addWord("Tablet", "Portable touchscreen device");
technology.addWord("Smartwatch", "Wearable computing device");

// Software category
technology.addWord("App", "Application program for devices");
technology.addWord("Browser", "Program for accessing the internet");
technology.addWord("Operating System", "Basic software managing computer hardware");
technology.addWord("Antivirus", "Program protecting against malicious software");

categories.push_back(technology);

Category travel("Travel");

// Transportation category
travel.addWord("Airplane", "Aircraft for air travel");
travel.addWord("Train", "Rail transport vehicle");
travel.addWord("Bus", "Large road vehicle for passengers");
travel.addWord("Taxi", "Car service with driver");

// Accommodation category
travel.addWord("Hotel", "Establishment providing paid lodging");
travel.addWord("Hostel", "Budget-friendly shared accommodation");
travel.addWord("Resort", "Place for relaxation and recreation");
travel.addWord("Motel", "Roadside hotel for motorists");

categories.push_back(travel);

Category sports("Sports");

// Team sports category
sports.addWord("Football", "Sport played with a ball between two teams");
sports.addWord("Basketball", "Sport played with a ball and hoops");
sports.addWord("Volleyball", "Sport played over a net");

// Individual sports category
sports.addWord("Swimming", "Sport of moving through water");
sports.addWord("Tennis", "Sport played with rackets");
sports.addWord("Golf", "Sport played with clubs and balls");

categories.push_back(sports);

        totalWords = animals.words.size() + professions.words.size() + fruits.words.size() + vegetables.words.size() + technology.words.size() + travel.words.size() +  sports.words.size();
    }

    void initializeStories() {

// Story 1
Story story1("The Maverick Woman",
    "Sophia had always been a maverick in her career. While many followed the traditional path, she evolved her own unique way of doing things.\n"
    "As a rebel kid, she had always questioned authority, and that spirit stayed with her into adulthood.\n"
    "Her colleagues found her nonchalant, never too worried about the opinions of others, but deep down, she knew she was vulnerable to the pressures of succeeding in a male-dominated industry.\n"
    "Despite the challenges, Sophia remained determined to push forward, carving her own path with confidence.\n");
stories.push_back(story1);

// Story 2
Story story2("The Courageous Decision",
    "Emily had always been the quiet one in the group, known for her gentle nature and thoughtful demeanor.\n"
    "Her friends often admired how she could stay calm, even in stressful situations.\n"
    "But beneath her composed exterior, Emily faced moments of uncertainty.\n"
    "When her company faced a major crisis, most of her colleagues panicked, but Emily made a bold decision to take charge.\n"
    "Though she felt uncertain at times, she found the strength to lead the team through the challenge.\n"
    "By the end of the month, her leadership had turned the situation around, earning her the respect of even the toughest critics.\n"
    "Emily learned that courage isn't about being fearless, but about making the right choices when it counts the most.\n");
stories.push_back(story2);

// Story 3
Story story3("The Hidden Kingdom",
    "Lena had always felt like she was meant for something greater.\n"
    "One evening, while exploring the forest near her home, she stumbled upon a glowing stone.\n"
    "When she touched it, the ground beneath her feet began to shake, and a hidden path appeared.\n"
    "Intrigued and nervous, she followed the path, which led her to an ancient, hidden kingdom.\n"
    "The kingdom was filled with creatures she’d only heard of in legends, like talking wolves and dragons that soared across the sky.\n"
    "The queen of the kingdom, a wise and kind woman, greeted Lena with open arms, telling her that she was destined to help them save their world from an impending darkness.\n"
    "As Lena began to uncover her magical powers, she realized her life was about to change in ways she never imagined.\n");
stories.push_back(story3);

    }

 void initializeQuestions() {
    // Questions for Story 1 ("The Maverick Woman")
    Question* q1 = new Question(
        "In 'The Maverick Woman,' what trait made Sophia stand out in her career?",
        {"She always followed traditional paths.",
         "She had a unique way of doing things.",
         "She avoided taking any risks.",
         "She only focused on the opinions of others."},
        1
    );
    allQuestions.push_back(q1);

    Question* q2 = new Question(
        "In 'The Maverick Woman,' how did Sophia feel about succeeding in a male-dominated industry?",
        {"Confident and unbothered.",
         "Vulnerable to the pressures of success.",
         "Indifferent about it.",
         "Completely uninterested in the industry."},
        1
    );
    allQuestions.push_back(q2);

    Question* q3 = new Question(
        "What does the word 'maverick' mean in the context of 'The Maverick Woman'?",
        {"A person who follows the crowd.",
         "A person who takes an independent stand.",
         "A person who dislikes change.",
         "A person who always agrees with others."},
        1
    );
    allQuestions.push_back(q3);

    Question* q4 = new Question(
        "What does 'vulnerable' mean in the context of 'The Maverick Woman'?",
        {"Strong and unbreakable.",
         "Susceptible to emotional or physical harm.",
         "Indifferent to challenges.",
         "Unaffected by external pressures."},
        1
    );
    allQuestions.push_back(q4);

    // Questions for Story 2 ("The Courageous Decision")
    Question* q5 = new Question(
        "What did Emily do during the crisis in her company in 'The Courageous Decision'?",
        {"She panicked along with her colleagues.",
         "She ignored the crisis and went on a vacation.",
         "She made a bold decision to take charge.",
         "She chose not to participate in the solution."},
        2
    );
    allQuestions.push_back(q5);

    Question* q6 = new Question(
        "In 'The Courageous Decision,' what quality did Emily demonstrate during the crisis?",
        {"Leadership and strength.",
         "Indifference to the situation.",
         "Fear and panic.",
         "Laziness and lack of initiative."},
        0
    );
    allQuestions.push_back(q6);

    Question* q7 = new Question(
        "In 'The Courageous Decision,' what does the word 'composed' mean?",
        {"Nervous and anxious.",
         "Calm and self-controlled.",
         "Angry and upset.",
         "Excited and energetic."},
        0
    );
    allQuestions.push_back(q7);

    // Questions for Story 3 ("The Hidden Kingdom")
    Question* q8 = new Question(
        "In 'The Hidden Kingdom,' what did Lena discover while exploring the forest?",
        {"A hidden treasure chest.",
         "A glowing stone.",
         "A magical creature.",
         "A secret portal to another world."},
        1
    );
    allQuestions.push_back(q8);

    Question* q9 = new Question(
        "In 'The Hidden Kingdom,' what creatures did Lena encounter in the hidden kingdom?",
        {"Unicorns and mermaids.",
         "Talking wolves and dragons.",
         "Fairies and trolls.",
         "Ghosts and vampires."},
        1
    );
    allQuestions.push_back(q9);

    Question* q10 = new Question(
        "What does 'intrigued' mean in the context of 'The Hidden Kingdom'?",
        {"Feeling uninterested.",
         "Feeling confused.",
         "Aroused curiosity or interest.",
         "Feeling scared."},
        2
    );
    allQuestions.push_back(q10);
}

public:
    LanguageLearningApp() : score(0), totalWords(0), wordsLearned(0) {

        initializeCategories();
        initializeStories();
        initializeQuestions();
    }

    ~LanguageLearningApp() {
        for (Question* q : allQuestions) {
            delete q;
        }
    }

    void displayMainMenu() {
        while (true) {
            system("cls"); // Clear screen
            cout << "\n=== Welcome to Language Learning App, " << userName << "! ===\n";
            cout << "1. Speak with ME\n";
            cout << "2. Listen and Practice\n";
            cout << "3. Mistakes\n";
            cout << "4. View Progress\n";
            cout << "5. Exit\n";
            cout << "Choose an option: ";

            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1: speakWithMe(); break;
                case 2: listenAndPractice(); break;
                case 3: reviewMistakes(); break;
                case 4: viewProgress(); break;
                case 5:
                    cout << "\nThank you for learning with us, " << userName << "!\n";
                    return;
                default:
                    cout << "Invalid choice! Press Enter to continue...";
                    cin.get();
            }
        }
    }

    void speakWithMe() {
        while (true) {
            system("cls");
            cout << "\n=== Categories ===\n";
            for (int i = 0; i < categories.size(); i++) {
                cout << i + 1 << ". " << categories[i].name << endl;
            }
            cout << "0. Back to Main Menu\n";
            cout << "Choose a category: ";

            int choice;
            cin >> choice;
            cin.ignore();

            if (choice == 0) break;
            if (choice > 0 && choice <= categories.size()) {
                categories[choice - 1].displayWords();
                cout << "\nPress Enter to continue...";
                cin.get();
            }
        }
    }

    void listenAndPractice() {
        // Show stories
        for (const Story& story : stories) {
            system("cls");
            story.display();
            cout << "\nPress Enter to continue...";
            cin.get();
        }

        // Ask if user wants to take quiz
        cout << "\nWould you like to take the quiz? (Y/N): ";
        char choice;
        cin >> choice;
        cin.ignore();

        if (toupper(choice) == 'Y') {
            takeQuiz();
        }
    }

    void takeQuiz() {
        score = 0;
        system("cls");
        cout << "\n=== Quiz Time ===\n";

        for (int i = 0; i < allQuestions.size(); i++) {
            Question* q = allQuestions[i];
            q->display(i + 1);

            cout << "Your answer (A/B/C/D): ";
            char answer;
            cin >> answer;
            cin.ignore();

            if (q->checkAnswer(answer)) {
                cout << "Correct!\n";
                score++;
            } else {
                cout << "Incorrect.\n";
                q->showCorrectAnswer();
                mistakeQueue.push(q);
            }
            cout << "\nPress Enter to continue...";
            cin.get();
        }

        cout << "\nQuiz completed! Your score: " << score << "/" << allQuestions.size() << endl;
        cout << "Press Enter to continue...";
        cin.get();
    }

    void reviewMistakes() {
        if (mistakeQueue.empty()) {
            cout << "\nNo mistakes to review!\n";
            cout << "Press Enter to continue...";
            cin.get();
            return;
        }

        cout << "\n=== Reviewing Mistakes ===\n";

        vector<Question*> remainingMistakes;
        while (!mistakeQueue.empty()) {
            Question* q = mistakeQueue.front();
            mistakeQueue.pop();

            q->display(1);
            cout << "Your answer (A/B/C/D): ";
            char answer;
            cin >> answer;
            cin.ignore();

            if (q->checkAnswer(answer)) {
                cout << "Correct!\n";
            } else {
                cout << "Still incorrect.\n";
                q->showCorrectAnswer();
                remainingMistakes.push_back(q);
            }
            cout << "\nPress Enter to continue...";
            cin.get();
        }

        // Put remaining mistakes back in queue
        for (Question* q : remainingMistakes) {
            mistakeQueue.push(q);
        }

        if (mistakeQueue.empty()) {
            cout << "\nCongratulations! You've corrected all your mistakes!\n";
        } else {
            cout << "\nKeep practicing! You have " << mistakeQueue.size()
                 << " mistakes left to review.\n";
        }
        cout << "Press Enter to continue...";
        cin.get();
    }

    void viewProgress() {
        system("cls");
        cout << "\n=== Progress Report for " << userName << " ===\n";
        cout << "Quiz Score: " << score << "/" << allQuestions.size() << endl;
        cout << "Mistakes to Review: " << mistakeQueue.size() << endl;

        cout << "\nPress Enter to continue...";
        cin.get();
    }
};

int main() {
    LanguageLearningApp app;
    app.displayMainMenu();
    return 0;
}
