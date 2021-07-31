
local brian = nil
local posx = 446
local posy = 260

function init()
end

function cleanup()
end

function _on_level_load(msg)
    brian = scene.new_brian(x, y)
   --[[ brian:set_supply({
      { typeid = Item.RED_FLASK , available = -1, price = 1 },
      { typeid = Item.BLUE_FLASK, available =  5, price = 1 }
    })
    --]]
    --brian:set_wellcome('brain.wellcome')
    --brian:set_goodbye('brian.goodbye')
end
