#include "../Templates/template.h"
#include <sys/socket.h>

// inet_addr
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>
#define PORT 8080
using namespace std;

Question::Question()
{
    questionBank.clear();
}

void Question::insertQuestion(QuestionInfo* question)
{
    this->questionBank.push_back(question);
}

void Question::getQuestion(int newSocket)
{
    int marks=0;
    int code;
    for(size_t i=0;i<questionBank.size();i++)
    {
        code = RECIEVE_QUESTION_CODE;
        send(newSocket,&code,sizeof(code),0);
        StudentQuestion *question = new StudentQuestion;
        strcpy(question->que, questionBank[i]->que);
        strcpy(question->opt1,questionBank[i]->opt1);
        strcpy(question->opt2,questionBank[i]->opt2);
        strcpy(question->opt3,questionBank[i]->opt3);
        strcpy(question->opt4,questionBank[i]->opt4);
        strcpy(question->marks,questionBank[i]->marks);
        send(newSocket, question,sizeof(* question),0);
        
        // receive answer of the question
        char answer[5];
        recv(newSocket, &answer,sizeof(* answer),0);
        if(answer[0] == questionBank[i]->answer[0])
        {
            marks += atoi(questionBank[i]->marks);
        }
    }
    code = END_EXAM_CODE;
    send(newSocket,&code,sizeof(code),0);
    sleep(1);
    send(newSocket,&marks,sizeof(marks),0);
    fstream file;
    file.open("result.txt",ios::app);
    file<<marks<<endl;
    file.close();

}

void server_side_student_registration(int newSocket)
{
    string uname, password, rollno, department;
    StudentUserInfo *newUserInfo = new StudentUserInfo;

    recv(newSocket, newUserInfo, sizeof(*newUserInfo), 0);
    uname = newUserInfo->username;
    password = newUserInfo->password;
    rollno = newUserInfo->rollno;
    department = newUserInfo->department;
    cout << "Received info..\n";
    fstream file;
    file.open("student_database.txt", ios::app);
    file << rollno << "|" << password << "|" << uname << "|" << department << "|"<<endl;
    file.close();
    return;
}

void server_side_teacher_registration(int newSocket)
{
    string uname, password, teacher_id, department;
    TeacherUserInfo *newUserInfo = new TeacherUserInfo;
    recv(newSocket, newUserInfo, sizeof(*newUserInfo), 0);

    uname = newUserInfo->username;
    password = newUserInfo->password;
    department = newUserInfo->department;
    teacher_id = newUserInfo->teacherid;

    fstream file;
    file.open("teacher_database.txt", ios::app);
    file << teacher_id << "|" << password << "|" << uname << "|" << department<<endl;
    file.close();
    return;
}

void server_side_login(int newSocket)
{
    cout<<"login started.."<<endl;
    string file_name;
    char usertype;
    recv(newSocket,&usertype,sizeof(usertype),0);
    
    if(usertype=='S')
    {
        file_name = "student_database.txt";
    }
    else
    {
        file_name = "teacher_database.txt";
    }

    loginInfo *userInfo = new loginInfo;
    int code;

    ifstream file(file_name, std::ios::in);
    if (file.is_open())
    {
        while (1)
        {
            file.seekg(0,ios::beg);
            recv(newSocket, userInfo, sizeof(*userInfo), 0);
            bool not_found = true;
            std::string line;
            while (std::getline(file, line))
            {
                string attr;
                stringstream str(line);
                getline(str,attr,'|');
                if(attr == userInfo->id)
                {
                    getline(str,attr,'|');
                    if(attr == userInfo->password)
                    {
                        not_found = false;
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if(not_found)
            {
                code = LOGIN_FAIL_CODE;
                send(newSocket,&code,sizeof(code),0);
            }
            else
            {
                break;
            }

        }
        file.close();
    }
    else
    {
        cout << "Error opening file" << std::endl;
    }
    return;
}

void setQuestion(int newSocket, string department, Question &deptObj)
{
    
    string fileName = department + ".txt";
    fstream file;
    file.open(fileName,ios::app);
    while(1)
    {
        int code;
        recv(newSocket,&code, sizeof(code),0);
        if(code == SET_QUESTION_CODE)
        {
            QuestionInfo* question = new QuestionInfo;
            recv(newSocket, question, sizeof(* question),0);
            deptObj.insertQuestion(question);
            file<<question->que<<"|"<<question->opt1<<"|"<<question->opt2<<"|"<<question->opt2<<"|"<<question->opt3<<"|"<<question->opt4<<"|"<<question->answer<<"|"<<question->marks<<"|"<<endl;
        }
        else
        {
            break;
        }
    }
    file.close();
    
}