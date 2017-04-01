//*****************************************************
// Projeto OpenGL2 + GLFW + Box2D
//
// Finalmente, removendo a GLUT da hist�ria :D
//
// Adaptado do projeto com GLUT por Rossana B. Queiroz 08/2011
// �ltima modifica��o: 30/03/2017
//*****************************************************

#include <cstdlib>
#include <iostream>
#include "Render.h" //� a Render que est� incluindo a glfw!
#include <ctime>

using namespace std;

//Algumas globais para interface e simula��o (IDEAL: criar uma classe gerenciadora)
float32 timeStep;
int32 velocityIterations;
int32 positionIterations;
float xMin = -40.0, xMax = 40.0, yMin = -40.0, yMax = 40.0; //ortho2D

// O objeto World serve para armazenar os dados da simula��o --> MUNDO F�SICO DA BOX2D
b2World *world;

//Alguns corpos r�gidos
b2Body *box1, *box2;
b2Body* ground;

//Objeto para a classe que faz o desenho das formas de colis�o dos corpos r�gidos
DebugDraw renderer;


//Callback de erro - PADR�O DA GLFW - n�o precisa mexer
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

//Callback de teclado - PADR�O DA GLFW - alterar conforme desejar (teclas de controle do programa)
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		
	}
}

//Rotina que cria uma caixa sempre com os mesmos atributos, variando s� a posi��o de seu centro
b2Body *CriaCaixa(float posX, float posY)
{
	b2Body *novoObjeto;

	//Primeiro, criamos a defini��o do corpo
	b2BodyDef bodyDef;
	bodyDef.position.Set(posX, posY);
	bodyDef.type = b2_dynamicBody;

	//Estamos usando uma forma de poligono, que pode ter at� 8 v�rtices
	b2PolygonShape forma;
	forma.SetAsBox(5, 5);

	//Depois, criamos uma fixture que vai conter a forma do corpo
	b2FixtureDef fix;
	fix.shape = &forma;
	//Setamos outras propriedades da fixture
	fix.density = 10.0;
	fix.friction = 0.5;
	fix.restitution = 0.5;

	//Por fim, criamos o corpo...
	novoObjeto = world->CreateBody(&bodyDef);
	//... e criamos a fixture do corpo 	
	novoObjeto->CreateFixture(&fix);

	return novoObjeto;
}


//Fun��o de inicializa��o da Box2D
void InitBox2D()
{

	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -9.8f);

	// Inicializa a biblioteca Box2D
	world = new b2World(gravity);

	// Define os par�metro para a simula��o
	// Quanto maior, mais preciso, por�m, mais lento - VALORES RECOMENDADOS no manual
	velocityIterations = 10;
	positionIterations = 8;
	timeStep = 1.0f / 60.0f; //60 Hz -- 1/60 seconds - pensando em tempo real para jogos (60FPS)

	//Cria��o de corpo r�gido: caixa
	box1 = CriaCaixa(-10, 10);

	//Cria��o de corpo r�gido: caixa
	box2 = CriaCaixa(10, 10);

	//Cria o ch�o
	//cria��o do corpo r�gido
	b2BodyDef bd;
	ground = world->CreateBody(&bd);
	//Forma do ch�o: edge
	b2EdgeShape shape;
	shape.Set(b2Vec2(-39.5, -39.5), b2Vec2(39.5, -39.5));
	ground->CreateFixture(&shape, 1.0);

}


// Fun��o de Execu��o da Simula��o
void RunBox2D()
{
	world->Step(timeStep, velocityIterations, positionIterations);
	world->ClearForces();
}

// Fun��o que imprime todos os objetos  
void PrintBodies()
{
	b2Body *b;
	float ang;
	b2Vec2 pos;
	//PERCORRE A LISTA DE CORPOS R�GIDOS DO MUNDO
	for (b = world->GetBodyList(); b; b = b->GetNext())
	{
		pos = b->GetPosition();
		ang = b->GetAngle();
		printf("%4.2f %4.2f %4.2f\n", pos.x, pos.y, ang);
	}
}

//Fun��o que chama as rotinas de desenho para cada fixture de um corpo r�gido, de acordo com o tipo da forma de colis�o dela e 
void DrawFixture(b2Fixture* fixture, b2Color color)
{
	const b2Transform& xf = fixture->GetBody()->GetTransform();

	switch (fixture->GetType())
	{
	case b2Shape::e_circle:
	{
		b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();

		b2Vec2 center = b2Mul(xf, circle->m_p);
		float32 radius = circle->m_radius;

		renderer.DrawCircle(center, radius, color);
	}
	break;

	case b2Shape::e_polygon:
	{
		b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
		int32 vertexCount = poly->m_count;
		b2Assert(vertexCount <= b2_maxPolygonVertices);
		b2Vec2 vertices[b2_maxPolygonVertices];

		for (int32 i = 0; i < vertexCount; ++i)
		{
			vertices[i] = b2Mul(xf, poly->m_vertices[i]);
		}

		renderer.DrawPolygon(vertices, vertexCount, color);
	}

	break;
	case b2Shape::e_edge:
	{
		b2EdgeShape* edge = (b2EdgeShape*)fixture->GetShape();
		int32 vertexCount;

		b2Vec2 vertices[b2_maxPolygonVertices];
		int i = 0;

		if (edge->m_hasVertex0)
		{
			vertices[i] = b2Mul(xf, edge->m_vertex0);
			i++;
		}
		vertices[i] = b2Mul(xf, edge->m_vertex1); i++;
		vertices[i] = b2Mul(xf, edge->m_vertex2); i++;
		if (edge->m_hasVertex3)
		{
			vertices[i] = b2Mul(xf, edge->m_vertex3);
			i++;
		}

		vertexCount = i;
		renderer.DrawPolygon(vertices, vertexCount, color);
	}

	break;

	}

}

//Para chamar a rotina de desenho das fixtures de um corpo
void DrawBody(b2Body *b, b2Color color)
{
	//Desenha todas as fixtures do corpo r�gido
	b2Fixture *f;
	for (f = b->GetFixtureList(); f; f = f->GetNext())
	{
		DrawFixture(f, color);
	}
}




//Rotina de render (n�o � mais callback)
void Render()
{
	b2Body *b;
	glColor3f(1, 0, 0);
	glPointSize(5);
	glLineWidth(3);

	b2Color color; 
	color.r = 1.0;
	color.g = 0.0;
	color.b = 0.0;

	//PERCORRE A LISTA DE CORPOS R�GIDOS DO MUNDO E CHAMA A ROTINA DE DESENHO PARA A LISTA DE FIXTURES DE CADA UM
	for (b = world->GetBodyList(); b; b = b->GetNext())
	{
		DrawBody(b, color);
	}
}

// Programa Principal 
int main(void)
{
	srand(time(0));

	//Inicializa��o da janela da aplica��o
	GLFWwindow* window;

	//Setando a callback de erro
	glfwSetErrorCallback(error_callback);

	//Inicializando a lib
	if (!glfwInit())
		exit(EXIT_FAILURE);

	//Criando a janela
	window = glfwCreateWindow(640, 480, "HELLO GLFW!! BYE BYE GLUT!!!", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//Colocando a janela criada como sendo o contexto atual
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	//Setando a callback de teclado
	glfwSetKeyCallback(window, key_callback);


	InitBox2D();


	while (!glfwWindowShouldClose(window)) //loop da aplica��o :)
	{
		float ratio;
		int width, height;

		//aqui recupera o tamanho atual da janela, para corre��o do aspect ratio mais tarde
		glfwGetFramebufferSize(window, &width, &height);
		//setando a viewport da OpenGL, para ocupar toda a janela da aplica��o
		glViewport(0, 0, width, height);

		// Limpa a janela de visualiza��o com a cor branca
		glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		//Setando a matriz de proje��o, para definir o Ortho2D (c�mera ortogr�fica 2D), respeitando o aspect ratio
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if (width >= height) 
		{
			ratio = width / (float)height;
			gluOrtho2D(xMin*ratio, xMax*ratio, yMin, yMax);
		}
		else
		{
			ratio = height / (float)width;
			gluOrtho2D(xMin, xMax, yMin*ratio, yMax*ratio);
		}
		
		//Setando a matriz de modelo, para mandar desenhar as primitivas
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Mandando simular e desenhar 	
		RunBox2D();
		Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}
