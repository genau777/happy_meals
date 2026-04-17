# HappyMeals Testing Guide

## How to Test the Application

### 1. Start the Application
```bash
cd ~/happymeals
./HappyMeals
```

### 2. Test Sign Up
- You should see the sign in page
- Click "Нет аккаунта? Зарегистрироваться"
- Enter: Login: `test`, Password: `test123`, Email: `test@test.com`
- Click "ЗАРЕГИСТРИРОВАТЬСЯ"
- You should be taken to the main hub

### 3. Test Recipe Search
- Click "🔎 НАЧАТЬ ПОИСК РЕЦЕПТА"
- **Step 1**: Select ingredients to EXCLUDE (e.g., select "Beef" and "Pork")
- Click "ДАЛЕЕ ➔"
- **Step 2**: Select cuisine (e.g., "Любая кухня") and type (e.g., "Любой тип")
- Click "ДАЛЕЕ ➔"
- **Step 3**: Set max time (e.g., 60 minutes)
- Click "🔍 НАЙТИ РЕЦЕПТ"

### 4. View Recipe Details
- You should see a list of recipes
- Click on any recipe (e.g., "Three Fish Pie")
- The right panel should show:
  - Recipe name and cuisine
  - Preparation time
  - **Full ingredient list with measurements** (e.g., "1kg Potatoes", "250g Salmon")
  - **Complete cooking instructions**

### 5. Test Favorites
- While viewing a recipe, click "⭐ ДОБАВИТЬ В ИЗБРАННОЕ"
- Button should change to "★ В ИЗБРАННОМ"
- Go back to main menu
- Click "⭐ ИЗБРАННЫЕ РЕЦЕПТЫ"
- You should see your saved recipe

### 6. Test Sign Out and Sign In
- Click "← ВЫЙТИ ИЗ АККАУНТА"
- You're back at sign in page
- Click "Уже есть аккаунт? Войти" (if on sign up page)
- Enter your credentials
- Click "ВОЙТИ В СИСТЕМУ"
- Your favorites should still be there!

## What Should Work

✅ **50 real recipes** from TheMealDB API
✅ **Full recipe details**: ingredients with measurements, cooking instructions
✅ **Ingredient filtering**: Exclude ingredients you don't want
✅ **Cuisine filtering**: Filter by cuisine type
✅ **Time filtering**: Find recipes under a certain time
✅ **Favorites**: Save and persist favorite recipes
✅ **Separate sign in/sign up pages**
✅ **User statistics**: View your search history

## Database Info

- **Dishes**: 50 recipes
- **Ingredients**: 264 unique ingredients
- **Cuisines**: 19 different cuisines (Russian, Japanese, Italian, Chinese, etc.)
- **All recipes include**: Full instructions, ingredient measurements, prep times

## Troubleshooting

If you don't see recipe details:
1. Make sure you're clicking on a recipe in the results list
2. Check that `data/dishes.sqlite` exists and is 150KB+
3. Try searching without any filters first

If no recipes are found:
1. Try not selecting any ingredients to exclude
2. Select "Любая кухня" for cuisine
3. Set time to 120 minutes

## Re-populate Database

If you want to fetch new recipes:
```bash
cd ~/happymeals
rm data/dishes.sqlite
python3 populate_recipes.py
```

This will fetch 50 new random recipes from TheMealDB.
