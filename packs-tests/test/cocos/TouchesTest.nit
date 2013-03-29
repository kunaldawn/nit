var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class Paddle	: cc.ScriptNode
{
	property position get _sprite.position set _sprite.position = value
	
	var _sprite = null;
	var _state = null;
	
	static PADDLE_GRABBED = 0
	static PADDLE_UNGRABBED = 1
	
	constructor()
	{
		base.constructor()

		touchEnabled = true;
		touchTargeted = true;
		
		channel().bind(EVT.CC_TARGETED_TOUCH_BEGIN, this, onTouchBegin)
		channel().bind(EVT.CC_TARGETED_TOUCH_MOVED, this, onTouchMove)
		channel().bind(EVT.CC_TARGETED_TOUCH_ENDED, this, onTouchEnded)
		
		_sprite = cc.Sprite( cocos.textureCache.addImage(IMG.PADDLE))
		addChild(_sprite)
		_state = PADDLE_UNGRABBED
	}
	
	function rect()
	{
		var s = _sprite.texture.contentSize;
		return cc.Rect(-s.width/2, -s.height/2, s.width, s.height)
	}
		
	function containsTouchLocation(touch)
	{
		return rect().contains(_sprite.touchToNodeAR(touch))
	}
	function onTouchBegin(evt: cc.TargetedTouchEvent)
	{
		print("begin")
		if (_state != PADDLE_UNGRABBED) return;
		if (containsTouchLocation(evt.touch))
		{
			evt.consume();
			_state = PADDLE_GRABBED
		}
	}
	
	function onTouchMove(evt: cc.TargetedTouchEvent)
	{
		print("move")
		var touchPoint = evt.touch.locationInView(evt.touch.view)
		touchPoint = cocos.director.toGl(touchPoint)
		
		_sprite.position = cc.Point(touchPoint.x, _sprite.position.y);
	}
	
	function onTouchEnded(evt: cc.TargetedTouchEvent)
	{
		print("rel")
		_state = PADDLE_UNGRABBED
	}
}

////////////////////////////////////////////////////////////////////////////////

class Ball
{
	property velocity		get _velocity  set _velocity = value
	property position 		get _sprite.position	set _sprite.position = value
	
	var _velocity = null;
	var _sprite = null;
	
	constructor(aTexture)
	{
		_sprite = cc.Sprite(aTexture);
	}

	function radius()
	{
		return _sprite.contentSize.width/2;
	}
	
	function move(delta)
	{
		//print("delta : "+delta)
		_sprite.position = _sprite.position - (_velocity * delta);
	
		if (_sprite.position.x > 320 - radius()) 
		{
			_sprite.position = cc.Point(320 - radius(), _sprite.position.y) ;
			_velocity.x *= -1;
		} 
		else if (_sprite.position.x < radius()) 
		{
			_sprite.position= cc.Point(radius(), _sprite.position.y) ;
			_velocity.x *= -1;
		}
	}
	
	function ccpToAngle(v)
	{
		return math.atan2(v.y, v.x)
	}
	
	function collideWithPaddle(paddle)
	{	
		var paddleRect = paddle.rect();
	
		var lowY = paddleRect.minY+paddle.position.y;
		var midY = (paddleRect.minY+paddleRect.maxY)/2+paddle.position.y;
		var highY = paddleRect.maxY+paddle.position.y;

		var leftX = paddleRect.minX+paddle.position.x;
		var rightX = paddleRect.maxX+paddle.position.x;
	
		if (_sprite.position.x > leftX && _sprite.position.x < rightX) 
		{
	
			var hit = false;
			var angleOffset = 0.0; 
		
			if (_sprite.position.y > midY && _sprite.position.y <= (highY+radius())) 
			{
				_sprite.position= cc.Point(_sprite.position.x, highY + radius());
				hit = true;
				angleOffset = math.PI / 2;
			}
			else if (_sprite.position.y < midY && _sprite.position.y >= (lowY-radius())) 
			{
				_sprite.position= cc.Point(_sprite.position.x, lowY - radius()) ;
				hit = true;
				angleOffset = -math.PI / 2;
			}
		
			if (hit) 
			{
				var hitAngle = ccpToAngle(paddle.position-_sprite.position) + angleOffset;
			
				var scalarVelocity = _velocity.length * 1.0;
				var velocityAngle = -ccpToAngle(_velocity) + 0.5 * hitAngle;
			
				_velocity = cc.Point(math.cos(velocityAngle), math.sin(velocityAngle))* scalarVelocity;
			}
		}	
	} 
}

////////////////////////////////////////////////////////////////////////////////

class PongLayer : cc.ScriptLayer
{
	_ball = null;
	_ballStartingVelocity = null;
	_paddles = null;
	
	constructor()
	{
		base.constructor()
		
		_ballStartingVelocity = cc.Point(20, -100)
		
		_ball = Ball(cocos.textureCache.addImage(IMG.BALL))
		_ball.position = cc.Point(160, 240);
		_ball.velocity = _ballStartingVelocity
		addChild(_ball._sprite);
		
		var paddleTexture = cocos.textureCache.addImage(IMG.PADDLE);
		_paddles = [];
		var paddle0 = Paddle();
		paddle0.position = cc.Point(160, 15);
		_paddles.push(paddle0);
		var paddle1 = Paddle();
		paddle1.position = cc.Point(160, 480-20-15);
		_paddles.push(paddle1);
		var paddle2 = Paddle();
		paddle2.position = cc.Point(160, 100);
		_paddles.push(paddle2);
		var paddle3 = Paddle();
		paddle3.position = cc.Point(160, 480-20-100);
		_paddles.push(paddle3);
		
		foreach (k, v in _paddles)
		{
			addChild(v);
		}
		
		cocos.director.timer.channel().bind(EVT.TICK, this, doStep);
	}
	
	function resetAndScoreBallForPlayer(player)
	{
		_ballStartingVelocity = _ballStartingVelocity * -1;
		_ball.velocity = _ballStartingVelocity;
		_ball.position = cc.Point(160, 240);
	}

	function doStep(evt: TimeEvent)
	{
		_ball.move(evt.delta);
		
		foreach (k, v in _paddles)
		{
			_ball.collideWithPaddle(v);
		}
		
		if (_ball.position.y > 480 - 20 + _ball.radius())
			resetAndScoreBallForPlayer(1)
		else if (_ball.position.y < -_ball.radius())
			resetAndScoreBallForPlayer(0)
	}
}

////////////////////////////////////////////////////////////////////////////////

class PongScene : TestScene
{
	constructor()
	{
		base.constructor()
		cocos.director :> deviceOrientation = ORIENT_PORTRAIT_UP;
	}
	
	function onReturnToMain()
	{
		cocos.director :> deviceOrientation = ORIENT_LANDSCAPE_LEFT;
		base.onReturnToMain();
	}
	
	function runThisTest()
	{
		var pLayer = PongLayer()
		addChild(pLayer);
		cocos.director.replaceScene(this)
	}
}

return PongScene()