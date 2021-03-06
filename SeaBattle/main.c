#include <stdlib.h>
#include <Windows.h>
#include "smart_console.h"
#include "battle_map.h"

#define KEY_ESC 27
#define KEY_ARROW_SPECIAL -32
#define KEY_UP 72
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_DOWN 80
#define KEY_ENTER 13

#define MAP_N_OFFSET_Y 1
#define MAP_1_OFFSET_X 1
#define MAP_2_OFFSET_X 30

void set_cursor_on_view_map(int x, int y);

int main(int argc, char** argv)
{
	char ch;
	int x, y, xc, yc;
	fire_result_t comp_fire;
	int exit = 0;
	int level = 1;
	battle_map_t* user_map;
	battle_map_t* computer_map;
	battle_map_view_t* user_map_view;
	battle_map_view_t* computer_map_view;

	init_console(0, 100);

	printf("Генерируется поле игрока... ");
	user_map = generate_random_map();
	if (user_map == NULL)
	{
		printf("ошибка!\nПопробуйте перезапустить игру");
		getch();
		return 1;
	}
	printf("готово!\n");

	printf("Генерируется поле компьютера... ");
	computer_map = generate_random_map();
	if (computer_map == NULL)
	{
		free_map(user_map);
		printf("ошибка!\nПопробуйте перезапустить игру");
		getch();
		return 1;
	}
	printf("готово!\n");

	printf("\nВыберите уровень сложности:\n");
	printf("  Безумный стрелок\n> Типичный противник\n  Нечестный бот\n");
	printf("\nENTER - выбор, ESC - выход");
	while ((ch = getch()) != KEY_ENTER)
	{
		if (ch == KEY_ESC)
		{
			free_map(user_map);
			free_map(computer_map);
			return 2;
		}

		if (ch == KEY_ARROW_SPECIAL)
		{
			ch = getch();
			switch (ch)
			{
			case KEY_UP:
				if (level > 0)
				{
					smart_print(0, 4 + level, ' ');
					smart_print(0, 4 + --level, '>');
				}
				break;

			case KEY_DOWN:
				if (level < 2)
				{
					smart_print(0, 4 + level, ' ');
					smart_print(0, 4 + ++level, '>');
				}
				break;
			}
		}
	}

	user_map_view = create_and_link_view(user_map, 0);
	computer_map_view = create_and_link_view(computer_map, 1);

	system("cls");

	print_canvas(&computer_map_view->view, MAP_1_OFFSET_X, MAP_N_OFFSET_Y);
	print_canvas(&user_map_view->view, MAP_2_OFFSET_X, MAP_N_OFFSET_Y);

	x = 4, y = 4;
	set_cursor_on_view_map(x, y);
	set_cursor_info(1, 100);

	while (!exit)
	{
		ch = getch();

		switch (ch)
		{
		case KEY_ESC:
			set_cursor_info(0, 100);
			system("cls");
			printf("Вы уверены, что хотите выйти? Прогресс игры будет утерян.\n");
			printf("ESC - вернуться в игру, ENTER - выйти из игры");
			do
			{
				ch = getch();
				if (ch == KEY_ESC)
				{
					system("cls");

					print_canvas(&computer_map_view->view, MAP_1_OFFSET_X, MAP_N_OFFSET_Y);
					print_canvas(&user_map_view->view, MAP_2_OFFSET_X, MAP_N_OFFSET_Y);
					
					set_cursor_on_view_map(x, y);
					set_cursor_info(1, 100);
				}
				else if (ch == KEY_ENTER)
				{
					exit = 1;
				}
			} while (ch != KEY_ENTER && ch != KEY_ESC);
			break;
		
		case KEY_ENTER:
			switch (fire(computer_map_view, x, y))
			{
			case FIRE_SUCCESS:
				if (!is_alive(computer_map_view))
				{
					set_cursor_info(0, 100);
					system("cls");
					printf("Победа!\n*Тут должен быть салют*\n");
					printf("ESC - выход");
					while (getch() != KEY_ESC) {}
					exit = 1;
					break;
				}

				print_canvas(&computer_map_view->view, MAP_1_OFFSET_X, MAP_N_OFFSET_Y);
				set_cursor_on_view_map(x, y);
				break;
			case FIRE_MISS:
				set_cursor_info(0, 100);
				print_canvas(&computer_map_view->view, MAP_1_OFFSET_X, MAP_N_OFFSET_Y);

				do
				{
					get_next_coord(user_map_view, level, &xc, &yc);
					comp_fire = fire(user_map_view, xc, yc);
					print_canvas(&user_map_view->view, MAP_2_OFFSET_X, MAP_N_OFFSET_Y);
					
					if (!is_alive(user_map_view))
					{
						system("cls");
						printf("Поражение!\nВ следующий раз повезет больше!\n");
						printf("Открытая карта противника - справа.\nESC - выход");
						merge_view_with_map(computer_map_view);
						print_canvas(&computer_map_view->view, 40, 1);
						while (getch() != KEY_ESC) {}
						exit = 1;
						break;
					}
					
					Sleep(500);
				} while (comp_fire == FIRE_SUCCESS);
				
				set_cursor_on_view_map(x, y);
				set_cursor_info(1, 100);
				break;
			case FIRE_DENY:
			default:
				break;
			}
			break;
		
		case KEY_ARROW_SPECIAL:
			ch = getch();
			switch (ch)
			{
			case KEY_UP:
				if (y > 0) set_cursor_on_view_map(x, --y);
				break;

			case KEY_LEFT:
				if (x > 0) set_cursor_on_view_map(--x, y);
				break;

			case KEY_RIGHT:
				if (x < BATTLE_MAP_SIZE_X - 1) set_cursor_on_view_map(++x, y);
				break;

			case KEY_DOWN:
				if (y < BATTLE_MAP_SIZE_Y - 1) set_cursor_on_view_map(x, ++y);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	free_view(user_map_view);
	free_view(computer_map_view);
	free_map(user_map);
	free_map(computer_map);

	return 0;
}

void set_cursor_on_view_map(int x, int y)
{
	set_cursor_pos(MAP_1_OFFSET_X + 2 + x * 2, MAP_N_OFFSET_Y + 2 + y * 2);
}
