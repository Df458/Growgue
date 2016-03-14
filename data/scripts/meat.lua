function apply()
    game.log("normal", "You eat the " .. this.name .. ".");
    game.log("EP Good", "That was a very good meal!.");
    player.ep = player.ep + 30;
end
