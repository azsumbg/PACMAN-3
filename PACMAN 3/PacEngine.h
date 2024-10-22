#pragma once

#ifdef PACENGINE_EXPORTS 
#define PAC_API __declspec(dllexport)
#else
#define PAC_API __declspec(dllimport)
#endif

#include <random>
#include <vector>

enum class dirs { up = 0, down = 1, left = 2, right = 3, stop = 4 };
enum class creatures { blue = 0, red = 1, pink = 2, orange = 3, hurt = 4, pacman = 5, no_type = 6 };

constexpr char up_flag = 0b00000001;
constexpr char down_flag = 0b00000010;
constexpr char left_flag = 0b00000100;
constexpr char right_flag = 0b00001000;

constexpr float scr_width{ 800.0f };
constexpr float scr_height{ 650.0f };
constexpr float ground{ 600.0f };
constexpr float sky{ 50.0f };

namespace gamedll
{
	class PAC_API RANDENGINE
	{
		private: 
			std::random_device rd{};
			std::vector<unsigned int> vSeeds;
			std::seed_seq* seed_seq_ptr = nullptr;
			std::mt19937* twister = nullptr;

		public:

			RANDENGINE();
			
			~RANDENGINE();

			int operator()(int min, int max);
	};

	class PAC_API ATOM
	{
		private:
			float width{};
			float height{};

		public:
			float x{};
			float y{};
			float ex{};
			float ey{};

			ATOM(float _x = 1.0f, float _y = 1.0f, float _width = 1.0f, float _height = 1.0f);
			virtual ~ATOM() {};

			float GetWidth() const;
			float GetHeight() const;

			void SetWidth(float new_width);
			void SetHeight(float new_height);

			void SetEdges();
			void NewDims(float new_width, float new_height);
	};

	class PAC_API ATOMPACK
	{
		private:
			ATOM* mPtr{ nullptr };
			int max_size{ 0 };
			int end_pos{ 0 };
			int next_pushback_pos{ 0 };

		public:

			ATOMPACK(int lenght) :max_size{ lenght }, mPtr{ new ATOM[lenght]{} } 
			{
				end_pos = max_size - 1;
			};
			virtual ~ATOMPACK()
			{
				if (mPtr)delete[]mPtr;
			}

			void push_back(ATOM& object);
			void push_front(ATOM& object);
			
			int size() const;
			int end() const;

			ATOM& operator[](int position) const;

	};

	class PAC_API CREATURES :public ATOM
	{
		protected:
			creatures type = creatures::no_type;
			creatures previous_type = creatures::no_type;

			int max_frames{ 1 };
			int current_frame{};
			int frame_delay = 5;

			int hurt_delay = 1000;

			float speed = 1.0f;

			CREATURES(float _x, float _y) :ATOM(_x, _y) {};

			char flags{ 0 };
			
		public:
			
			dirs dir = dirs::stop;
			bool panic = false;

			virtual ~CREATURES() {};
			virtual void Release() = 0;

			virtual void Move(float gear, dirs to_where, dirs alternative_dir, ATOMPACK& Obstacles) = 0;
			virtual bool Hurt() = 0;
			virtual int GetFrame() = 0;
			
			virtual creatures GetType() const = 0;
			
			void SetFlag(char which_flag);
			void ResetFlag(char which_flag);
			bool GetFlag(char which_flag) const;
	};

	typedef CREATURES* Creature;

	extern PAC_API Creature Factory(creatures what, float startX, float startY);

	extern PAC_API ATOMPACK& InitObstaclesPack(int maximum_size);
}

