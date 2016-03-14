function apply()
    game.log("default", "You eat the " .. this.name .. ".");
    game.log("HP good", "You feel much healthier!");
    player.hp = player.hp + 50;
end
