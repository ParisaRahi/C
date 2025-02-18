/*
 * This file implements two functions that read XML and binary information from a buffer,
 * respectively, and return pointers to Record or NULL.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "recordFromFormat.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
 


static int get_course_index(const char *course)
{
    if (strcmp(course, "IN1000") == 0)
        return Course_IN1000;
    if (strcmp(course, "IN1010") == 0)
        return Course_IN1010;
    if (strcmp(course, "IN1020") == 0)
        return Course_IN1020;
    if (strcmp(course, "IN1030") == 0)
        return Course_IN1030;
    if (strcmp(course, "IN1050") == 0)
        return Course_IN1050;
    if (strcmp(course, "IN1060") == 0)
        return Course_IN1060;
    if (strcmp(course, "IN1080") == 0)
        return Course_IN1080;
    if (strcmp(course, "IN1140") == 0)
        return Course_IN1140;
    if (strcmp(course, "IN1150") == 0)
        return Course_IN1150;
    if (strcmp(course, "IN1900") == 0)
        return Course_IN1900;
    if (strcmp(course, "IN1910") == 0)
        return Course_IN1910;
    return -1;
}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Helper function to find a specific tag
const char *find_tag(const char *buffer, const char *tag, const char **end)
{
    char start_tag[32];
    snprintf(start_tag, sizeof(start_tag), "<%s", tag); // changed here
    const char *end_tag = " />";                        // changed here

    const char *start = strstr(buffer, start_tag);
    if (!start)
        return NULL;

    // Move start to point to the attribute value
    start = strchr(start, '\"'); // Look for the first quote
    if (!start)
        return NULL; // Didn't find quote, malformed tag
    start++;         // Move past the quote to the start of the value

    *end = strstr(start, end_tag);
    if (!*end)
        return NULL; // Didn't find the end tag, malformed tag

    return start;
}

Grade stringToGrade(char *str)
{
    if (strcmp(str, "Bachelor") == 0)
        return Grade_Bachelor;
    else if (strcmp(str, "Master") == 0)
        return Grade_Master;
    else if (strcmp(str, "PhD") == 0)
        return Grade_PhD;
    else
        return Grade_None;
}



Record *XMLtoRecord(char *buffer, int bufSize, int *bytesRead)
{

     //printf("\n record is %s \n", buffer);


    if (!buffer)
    {
        *bytesRead = 0;
        fprintf(stderr, "buffer is null\n");
        return NULL;
    }

    if (bufSize < 1)
    {
        *bytesRead = 0;
        return NULL;
    }

    Record *r = newRecord();
    const char *start, *end;

    start = find_tag(buffer, "source", &end);
    if (start && end)
    {
        setSource(r, start[0]);
    }

    start = find_tag(buffer, "dest", &end);
    if (start && end)
    {
        setDest(r, start[0]);
    }

    start = find_tag(buffer, "username", &end);
    if (start && end)
    {
        char *username = (char *)malloc(100000);
        strncpy(username, start, end - start - 1);
        username[end - start - 1] = '\0';
        setUsername(r, username);
        free(username);
    }

    start = find_tag(buffer, "id", &end);
    if (start && end)
    {
        setId(r, atoi(start));
    }

    start = find_tag(buffer, "group", &end);
    if (start && end)
    {
        setGroup(r, atoi(start));
    }

    start = find_tag(buffer, "semester", &end);
    if (start && end)
    {
        setSemester(r, atoi(start));
    }

    start = find_tag(buffer, "grade", &end);
    if (start && end)
    {
        char *gradeStr = (char *)malloc(256);
        strncpy(gradeStr, start, end - start - 1);
        gradeStr[end - start] = '\0';
        setGrade(r, stringToGrade(gradeStr));
        free(gradeStr);
    }

    start = end;
    // r->has_courses = false;
    while ((start = find_tag(start, "course", &end)) != NULL)
    {
        if (start && end)
        {
            char *codeStr = (char *)malloc(256);
            strncpy(codeStr, start, end - start - 1);
            codeStr[end - start] = '\0';

            int courseIndex = get_course_index(codeStr);

            if (courseIndex >= 0)
            {
                setCourse(r, courseIndex);
            }

            // Move the start pointer after the current course tag
            start = end;
            free(codeStr);
        }
    }

    *bytesRead = bufSize;

    return r;
}

Record *BinaryToRecord(char *buffer, int bufSize, int *bytesread)
{
    if (bufSize < 1)
    {
        *bytesread = 0;
        return NULL;
    }

    uint8_t flags = buffer[0];

    int idx = 1;
    Record *r = newRecord();

    if (flags & FLAG_SRC)
    {
        if (idx >= bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        setSource(r, buffer[idx]);
        idx++;
    }

    if (flags & FLAG_DST)
    {
        if (idx >= bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        setDest(r, buffer[idx]);
        idx++;
    }

    if (flags & FLAG_USERNAME)
    {
        if (idx + 4 > bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }

        int usernameLength = ntohl(*(uint32_t *)&buffer[idx]);

        idx += 4;

        if (idx + usernameLength > bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }

        char *username = (char *)malloc(usernameLength + 1);
        strncpy(username, &buffer[idx], usernameLength);
        username[usernameLength] = '\0';

        setUsername(r, username);
        free(username);
        idx += usernameLength;
    }

    if (flags & FLAG_ID)
    {
        if (idx + 4 > bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        uint32_t id = ntohl(*(uint32_t *)&buffer[idx]);

        setId(r, id);
        idx += 4;
    }

    if (flags & FLAG_GROUP)
    {
        if (idx + 4 > bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        uint32_t group = ntohl(*(uint32_t *)&buffer[idx]);
        setGroup(r, group);
        idx += 4;
    }

    if (flags & FLAG_SEMESTER)
    {
        if (idx >= bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        setSemester(r, buffer[idx]);
        idx++;
    }

    if (flags & FLAG_GRADE)
    {
        if (idx >= bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        setGrade(r, (Grade)buffer[idx]);
        idx++;
    }

    if (flags & FLAG_COURSES)
    {
        if (idx + 2 > bufSize)
        {
            deleteRecord(r);
            *bytesread = 0;
            return NULL;
        }
        uint16_t courses = ntohs(*(uint16_t *)&buffer[idx]);
        idx += 2;      

        // Define the course flag array
        uint16_t course_flags[11] = {Course_IN1000, Course_IN1010, Course_IN1020, Course_IN1030, Course_IN1050,
                                     Course_IN1060, Course_IN1080, Course_IN1140, Course_IN1150, Course_IN1900,
                                     Course_IN1910};

        for (int i = 0; i < 11; i++)
        {
            if (courses & (1 << i))
            {
                setCourse(r, course_flags[i]);
            }
        }
    }

    *bytesread = idx;
    return r;
}
