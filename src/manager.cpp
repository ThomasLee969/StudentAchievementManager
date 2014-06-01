#include "manager.h"

namespace SAM {

Manager::Manager() : students_(),
                     courses_()
{
}

bool Manager::AddStudent(const BasicStudentInfo &student_info)
{
    if (students_.count(student_info.id) != 0)  // id has been occupied
        return false;

    students_.emplace(student_info.id, Student(student_info));
    return true;
}

bool Manager::RemoveStudent(Student::IDType student_id)
{
    auto iter = students_.find(student_id);
    if (iter == students_.end())  // student not exist
        return false;

    for (Course::IDType course_id : iter->second.courses_taken())
    {
        courses_[course_id].RemoveStudent(iter->second);
    }
    students_.erase(iter);
    return true;
}

bool Manager::HasStudent(Student::IDType student_id) const
{
    return students_.count(student_id) != 0;
}

const Student * Manager::SearchStudent(Student::IDType student_id) const
{
    auto iter = students_.find(student_id);
    if (iter == students_.end())
        return nullptr;
    else
        return &(iter->second);
}

bool Manager::SetStudentBasicInfo(Student::IDType student_id,
                                  const BasicStudentInfo &info)
{
    auto iter = students_.find(student_id);
    if (iter == students_.end())
        return false;

    if (student_id != info.id)  // the id has changed
    {
        if (students_.count(info.id) != 0)  // id already been taken
            return false;

        Student new_student(info);
        // updating IDs
        for (Course::IDType course_id : iter->second.courses_taken())
        {
            // save scores
            auto scores_before = courses_[course_id].LookUpScore(student_id);
            courses_[course_id].RemoveStudent(iter->second);
            courses_[course_id].AddStudent(new_student);
            // recover scores
            for (std::size_t exam_index = 0; exam_index < scores_before.size();
                 exam_index++)
            {
                courses_[course_id].ModifyScore(info.id,
                                                scores_before[exam_index],
                                                scores_before[exam_index]);
            }
        }

        students_.erase(iter);
        students_.emplace(info.id, new_student);
    }
    else
    {
        iter->second.set_basic_info(info);
    }

    return true;
}

bool Manager::AddCourse(const BasicCourseInfo &info)
{
    if (students_.count(info.id) != 0)  // id has been occupied
        return false;

    courses_.emplace(info.id, Course(info));
    return true;
}

bool Manager::RemoveCourse(Course::IDType course_id)
{
    auto iter = courses_.find(course_id);
    if (iter == courses_.end())  // course not found
        return false;

    for (Student::IDType student_id : iter->second.StudentList())
    {
        students_[student_id].RemoveCourse(course_id);
    }
    return true;
}

bool Manager::HasCourse(Course::IDType course_id) const
{
    return courses_.count(course_id) != 0;
}

const Course * Manager::SearchCourse(Course::IDType course_id) const
{
    auto iter = courses_.find(course_id);
    if (iter == courses_.end())
        return nullptr;

    return &(iter->second);
}

bool Manager::SetCourseBasicInfo(Course::IDType course_id,
                                 const BasicCourseInfo &info)
{
    auto iter = courses_.find(course_id);
    if (iter == courses_.end())
        return false;

    if (course_id != info.id)
    {
        if (courses_.count(info.id) != 0)
            return false;  // new id has been taken

        Course new_course(iter->second);
        new_course.set_basic_info(info);
        // updating IDs
        for (auto student_id : new_course.StudentList())
        {
            students_[student_id].RemoveCourse(course_id);
            students_[student_id].AddCourse(info.id);
        }

        courses_.erase(iter);
        courses_.emplace(info.id, new_course);
    }
    else
    {
        iter->second.set_basic_info(info);
    }

    return true;
}

bool Manager::AddStudentToCourse(Student::IDType student_id,
                                 Course::IDType course_id)
{
    auto iter_student = students_.find(student_id);
    auto iter_course = courses_.find(course_id);

    if (iter_student == students_.end() ||
        iter_course == courses_.end())
        return false;

    iter_course->second.AddStudent(iter_student->second);
    return true;
}

bool Manager::AddStudentToCourse(
        const std::vector<Student::IDType> &student_ids,
        Course::IDType course_id)
{
    auto iter_course = courses_.find(course_id);

    if (iter_course == courses_.end())
        return false;

    bool at_least_one_student_id_is_valid = false;
    for (auto student_id : student_ids)
    {
        auto iter_student = students_.find(student_id);
        if (iter_student != students_.end())
        {
            iter_course->second.AddStudent(iter_student->second);
            at_least_one_student_id_is_valid = true;
        }
    }

    return at_least_one_student_id_is_valid;
}

bool Manager::RemoveStudentFromCourse(Student::IDType student_id,
                                      Course::IDType course_id)
{
    auto iter_student = students_.find(student_id);
    auto iter_course = courses_.find(course_id);

    if (iter_student == students_.end() ||
        iter_course == courses_.end())
        return false;

    iter_course->second.RemoveStudent(iter_student->second);
    return true;
}

bool Manager::AddExam(const Exam &exam,
                      std::vector<Student::IDType> &unscored_students)
{
    auto iter = courses_.find(exam.course_id);
    if (iter == courses_.end())
        return false;

    iter->second.AddExam(exam, unscored_students);
    return true;
}

bool Manager::RemoveExam(Course::IDType course_id, std::size_t exam_index)
{
    auto iter = courses_.find(course_id);
    if (iter == courses_.end())
        return false;

    return iter->second.RemoveExam(exam_index);
}

bool Manager::ModifyScore(Student::IDType student_id, Course::IDType course_id,
                          std::size_t exam_index, Exam::ScoreType new_score)
{
    auto iter = courses_.find(course_id);
    if (iter == courses_.end())
        return false;

    return iter->second.ModifyScore(student_id, exam_index, new_score);
}



}  // namespace SAM