function apply()
    game.log("default", "You eat the " .. this.name .. ".");
    game.log("default", "It's not very filling, but you feel like you made a healthy choice.");
    player.hp = player.hp + 10;
    player.ep = player.ep + 5;
end
