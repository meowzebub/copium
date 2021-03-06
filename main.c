#include <stdio.h>
#include "sys/wait.h"
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <libavutil/log.h>

#include "library.h"
#include "gui.h"

char *convert_sec_to_minsec(int time);

int main(void)
{
    int fd1[2];
    int fd2[2];
    int fd3[2];

    pipe(fd1);
    pipe(fd2);
    pipe(fd3);

    int album_len;

    int quit = false;
    char *asc_desc[] = {"Ascending", "Descending"};
    char *sort_data[] = {"Title", "Artist", "Duration"};
    char input[100] = {'\0'};

    av_log_set_level(AV_LOG_QUIET);

    int f = fork();

    if (f)
    {
        fcntl(fd2[1], F_SETFL, O_NONBLOCK);
        fcntl(fd3[0], F_SETFL, O_NONBLOCK);
        close(fd1[0]);

        struct library *library = make_library();

        // 0 - Albums
        // 1 - Tracks
        // 2 - Track info
        // 3 - Bottom bar
        WINDOW *ctrl_win[NUM_WIN];
        setup(ctrl_win);

        int fd = open("library.data", O_RDONLY);
        if (fd == -1)
        {
            create_input_window("Please enter a path to a song to add to the library below. (relative please)", input);
            library = add_to_library(library, input);
        }
        else
        {
            library = read_library();
        }

        struct album *queue = make_album();

        int choice;
        int highlight_album = 0;
        int highlight_track = 0;

        struct album *curr_track = NULL;
        struct album *play_track = NULL;

        int i;
        while (TRUE)
        {
            if (quit)
            {
                kill(f, SIGKILL);
                break;
            }

            werase(ctrl_win[0]);
            box(ctrl_win[0], 0, 0);
            werase(ctrl_win[1]);
            box(ctrl_win[1], 0, 0);
            werase(ctrl_win[2]);
            box(ctrl_win[2], 0, 0);
            werase(ctrl_win[3]);
            box(ctrl_win[3], 0, 0);

            int status;
            int library_len = get_library_len(library);

            i = 0;
            for (struct library *temp = library; temp; temp = temp->next, ++i)
            {
                if (i == highlight_album)
                    wattron(ctrl_win[0], A_REVERSE);
                mvwprintw(ctrl_win[0], i + 2, 2, temp->album->name);
                wattroff(ctrl_win[0], A_REVERSE);
            }

            struct library *curr_album = get_nth_album(library, highlight_album);

            i = 0;
            for (struct album *temp = curr_album->album; temp; temp = temp->next, ++i)
            {
                mvwprintw(ctrl_win[1], i + 2, 2, temp->data->title);
            }

            char strtime[10] = {"/0"};
            if (read(fd3[0], strtime, 10) != -1 && play_track != NULL)
            {
                int bar_y, bar_x;
                getmaxyx(ctrl_win[3], bar_y, bar_x);

                char title[512];
                if (queue->data == NULL)
                {
                    sprintf(title, "%s by %s", play_track->data->title, play_track->data->artist);
                    mvwprintw(ctrl_win[3], 2, (bar_x - strlen(title)) / 2, title);
                    mvwprintw(ctrl_win[3], 4, 2, "%s / %s", convert_sec_to_minsec(atoi(strtime)), convert_sec_to_minsec(play_track->data->duration));

                    float ratio = atoi(strtime) / (float)play_track->data->duration;

                    for (int j = 0; j < bar_x - 23; j++)
                    {
                        int threshold = ratio * (bar_x - 25);
                        if (j < threshold)
                        {
                            wattron(ctrl_win[3], A_REVERSE);
                            mvwprintw(ctrl_win[3], 4, 15 + j, "#");
                            wattroff(ctrl_win[3], A_REVERSE);
                        }
                        else
                            mvwprintw(ctrl_win[3], 4, 15 + j, "#");
                    }
                }
                else
                {
                    sprintf(title, "%s by %s", queue->data->title, queue->data->artist);
                    mvwprintw(ctrl_win[3], 2, (bar_x - strlen(title)) / 2, title);
                    mvwprintw(ctrl_win[3], 4, 2, "%s / %s", convert_sec_to_minsec(atoi(strtime)), convert_sec_to_minsec(queue->data->duration));

                    float ratio = atoi(strtime) / (float)queue->data->duration;

                    for (int j = 0; j < bar_x - 23; j++)
                    {
                        int threshold = ratio * (bar_x - 25);
                        if (j < threshold)
                        {
                            wattron(ctrl_win[3], A_REVERSE);
                            mvwprintw(ctrl_win[3], 4, 15 + j, "#");
                            wattroff(ctrl_win[3], A_REVERSE);
                        }
                        else
                            mvwprintw(ctrl_win[3], 4, 15 + j, "#");
                    }
                }
            }

            wrefresh(ctrl_win[0]);
            wrefresh(ctrl_win[1]);
            wrefresh(ctrl_win[2]);
            wrefresh(ctrl_win[3]);
            choice = wgetch(ctrl_win[0]);
            nodelay(ctrl_win[0], TRUE);
            nodelay(ctrl_win[3], TRUE);

            int selection1;
            int selection2;
            switch (choice)
            {
            case KEY_UP:
                if (!(highlight_album <= 0))
                    --highlight_album;
                break;
            case KEY_DOWN:
                if (!(highlight_album >= library_len - 1))
                    ++highlight_album;
                break;
            case 'a':
                memset(input, '\0', sizeof(input));
                create_input_window("Please enter a path to a song to add to the library below. (relative please)", input);
                library = add_to_library(library, input);
                break;
            case 'q':
                quit = true;
                break;
            case 's':
                selection1 = create_selection_window(asc_desc, "ascending or descending", 2);
                library = sort(library, selection1);
                break;
            case 'w':
                library = save_library(library);
                create_status_window("Saved library data to \"library.data\".");
                break;
            case 32:
                if (waitpid(f, &status, WNOHANG | WUNTRACED))
                    kill(f, SIGCONT);
                else
                    kill(f, SIGSTOP);
                break;
            case 62:
                write(fd2[1], "62", 10);
                if (queue->next != NULL)
                {
                    play_track = queue->next;
                    queue = queue->next;
                }
                break;
            case KEY_RIGHT:
                highlight_track = 0;
                while (TRUE)
                {
                    album_len = get_album_len(curr_album->album);
                    curr_track = get_nth_track(curr_album->album, highlight_track);

                    i = 0;
                    for (struct album *temp = curr_album->album; temp; temp = temp->next, ++i)
                    {
                        if (i == highlight_track)
                            wattron(ctrl_win[1], A_REVERSE);
                        mvwprintw(ctrl_win[1], i + 2, 2, temp->data->title);
                        wattroff(ctrl_win[1], A_REVERSE);
                    }

                    werase(ctrl_win[2]);
                    box(ctrl_win[2], 0, 0);
                    mvwprintw(ctrl_win[2], 2, 2, "Title: %s", curr_track->data->title);
                    mvwprintw(ctrl_win[2], 3, 2, "Artist: %s", curr_track->data->artist);
                    mvwprintw(ctrl_win[2], 4, 2, "Album: %s", curr_track->data->album);
                    mvwprintw(ctrl_win[2], 5, 2, "Genre: %s", curr_track->data->genre);
                    mvwprintw(ctrl_win[2], 6, 2, "Date Created: %s", curr_track->data->date);
                    mvwprintw(ctrl_win[2], 7, 2, "Duration: %s", convert_sec_to_minsec(curr_track->data->duration));

                    wrefresh(ctrl_win[1]);
                    wrefresh(ctrl_win[2]);
                    wrefresh(ctrl_win[3]);
                    choice = wgetch(ctrl_win[1]);

                    if (choice == KEY_LEFT)
                    {
                        i = 0;
                        for (struct album *temp = curr_album->album; temp; temp = temp->next, ++i)
                        {
                            if (i == highlight_track)
                                mvwprintw(ctrl_win[1], i + 2, 2, temp->data->title);
                        }

                        break;
                    }

                    if (choice == 'q')
                    {
                        quit = true;
                        break;
                    }

                    switch (choice)
                    {
                    case KEY_UP:
                        if (!(highlight_track <= 0))
                            --highlight_track;
                        break;
                    case KEY_DOWN:
                        if (highlight_track < album_len - 1)
                            ++highlight_track;
                        break;
                    case 's':
                        selection1 = create_selection_window(sort_data, "track title, artist, or duration", 3);
                        selection2 = create_selection_window(asc_desc, "ascending or descending", 2);
                        werase(ctrl_win[1]);
                        box(ctrl_win[1], 0, 0);
                        i = 0;
                        for (struct album *temp = curr_album->album; temp; temp = temp->next, ++i)
                        {
                            if (i == highlight_track)
                                wattron(ctrl_win[1], A_REVERSE);
                            mvwprintw(ctrl_win[1], i + 2, 2, temp->data->title);
                            wattroff(ctrl_win[1], A_REVERSE);
                        }
                        curr_album = sort_album(curr_album, selection2, selection1);

                    case 32:
                        if (waitpid(f, &status, WNOHANG | WUNTRACED))
                            kill(f, SIGCONT);
                        else
                            kill(f, SIGSTOP);
                        break;
                    case 62:
                        write(fd2[1], "62", 10);
                        if (queue->next != NULL)
                        {
                            play_track = queue->next;
                            queue = queue->next;
                        }
                        break;
                    case 10:
                    add_to_album(queue, curr_track->data);
                        write(fd1[1], curr_track->data->path, sizeof(curr_track->data->path));
                        play_track = curr_track;
                        kill(f, SIGCONT);
                        break;
                    default:
                        break;
                    }
                }
            default:
                break;
            }
        }

        endwin();
    }
    else
    {
        struct timespec start, end;
        int elapsed;
        fcntl(fd2[0], F_SETFL, O_NONBLOCK);
        fcntl(fd3[1], F_SETFL, O_NONBLOCK);
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            printf("SDL could not initialise. Error:%s\n", SDL_GetError());
            return -1;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            printf("SDL_mixer could not initialise. Error: %s\n", Mix_GetError());
            return -1;
        }

        while (true)
        {
            char buffer[100] = {'\0'};
            read(fd1[0], buffer, sizeof(buffer));

            Mix_Music *music = NULL;
            music = Mix_LoadMUS(buffer);
            if (music == NULL)
            {
                printf("Failed to load music. Error: %s\n", Mix_GetError());
                return -1;
            }

            int i = Mix_PlayMusic(music, 1);
            Mix_VolumeMusic(10);
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
            int running = TRUE;
            char status[10] = {'\0'};
            while (read(fd2[0], status, sizeof(status)) != -1)
            {
            }
            while (running)
            {
                char status[10] = {'\0'};
                read(fd2[0], status, sizeof(status));
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                elapsed = end.tv_sec - start.tv_sec;
                char time2[10];
                sprintf(time2, "%d", elapsed);
                write(fd3[1], time2, 10);
                if (!Mix_PlayingMusic())
                    break;
                if (!strcmp(status, "62"))
                    Mix_HaltMusic();
            }
        }

        Mix_CloseAudio();
        SDL_Quit();
    }

    printf("%d\n", album_len);

    return 0;
}

char *convert_sec_to_minsec(int time)
{
    int minutes = time / 60;
    int seconds = time % 60;

    char *formatted_time = calloc(10, sizeof(char));
    if (seconds < 10)
        snprintf(formatted_time, 10, "%d:0%d", minutes, seconds);
    else
        snprintf(formatted_time, 10, "%d:%d", minutes, seconds);

    return formatted_time;
}
