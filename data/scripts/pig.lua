function update()
    if math.random(50) == 1 then
        game.log("default", this.name .. ": Oink!");
    elseif math.random(50) == 1 then
        game.log("default", this.name .. ": *Snort*");
    end
end
