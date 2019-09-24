#include "MiniEngine.h"
#include <map>
#include <set>

namespace engine {

	Input::Input(const char* name/*, const std::vector<Node* >& inputs*/)
	{
		_name = name;
	}

	void Input::forward(const Eigen::MatrixXf& value) {
		_value = value;
	}

	void Input::backward() {
		for (auto node : _outputs) {
			_gradients[this] = node->getGradient(this);
		}
	}

	Linear::Linear(Node* nodes, Node* weights, Node* bias)
		:Node(std::vector<Node*>({nodes,weights,bias}))
		,_nodes(nodes)
		,_weights(weights)
		,_bias(bias)
	{
		_name = "Linear";
	}

	void Linear::forward(const Eigen::MatrixXf& )
	{
		_value = _nodes->getValue() * _weights->getValue() + _bias->getValue();
	}

	void Linear::backward()
	{
		for (auto node : _outputs) {
			auto grad = node->getGradient(this);
			_gradients[_weights] = _nodes->getValue().transpose() * grad;
			//�������,��Ϊһ��
			_gradients[_bias] = grad.colwise().sum();
			_gradients[_nodes] = grad * _weights->getValue().transpose();
		}
	}

	Sigmoid::Sigmoid(Node* node)
		:Node(std::vector<Node*>({node}))
		,_node(node)
	{
		_name = "Sigmoid";
	}

	void Sigmoid::forward(const Eigen::MatrixXf& )
	{
		_value = _impl(_node->getValue());
	}

	void Sigmoid::backward()
	{
		auto y = _value;
		_partial = y-y*y;

		for (auto node : _outputs) {
			auto grad = node->getGradient(this);
			_gradients[_node] = grad * _partial;
		}
	}

	Eigen::MatrixXf Sigmoid::_impl(const Eigen::MatrixXf& x)
	{
		return (x.array().exp() + 1).inverse();
	}

	MSE::MSE(Node* y, Node* y_hat)
		:Node(std::vector<Node*>({y,y_hat}))
		,_y(y),_y_hat(y_hat)
	{
		_name = "MSE";
	}

	void MSE::forward(const Eigen::MatrixXf& value)
	{
		_diff = _y->getValue() - _y_hat->getValue();
		auto v = Eigen::MatrixXf(1, 1);
		v<<(_diff * _diff).mean();
		_value = v;
	}

	void MSE::backward()
	{
		auto r = _y_hat->getValue().rows();
		_gradients[_y] = _diff * (2 / r);
		_gradients[_y_hat] = _diff * (-2 / r);
	}


	std::vector<Node*> topological_sort(Node* input_nodes)
	{
		//���ݴ�������ݳ�ʼ��ͼ�ṹ
		Node* pNode = nullptr;
		//pair��һ��Ϊ����,�ڶ���Ϊ���
		std::map < Node*, std::pair<std::set<Node*>, std::set<Node*> > > g;
		//����������Χ�ڵ�
		std::list<Node*> vNodes;
		vNodes.emplace_back(input_nodes);
		//��ȱ���,�ȱ�������ڵ�,�ٱ�������ڵ�
		//�Ѿ��������Ľڵ�
		std::set<Node*> sVisited;
		while (vNodes.size() && (pNode = vNodes.front())) {
			if (sVisited.find(pNode) != sVisited.end()) vNodes.pop_front();
			const auto& outputs = pNode->getOutputs();
			for (auto item: outputs)
			{
				g[pNode].second.insert(item);	//���itemΪpnode������ڵ�
				g[item].first.insert(pNode);	//���pnodeΪitem������ڵ�
				if(sVisited.find(item)==sVisited.end()) vNodes.emplace_back(item);	//��û�з��ʹ��Ľڵ���ӵ������ʶ�����
			}
			const auto& inputs = pNode->getInputs();
			for (auto item: inputs)
			{
				g[pNode].first.insert(item);	//���itemΪpnode������ڵ�
				g[item].second.insert(pNode);	//���pnodeΪitem������ڵ�
				if (sVisited.find(item) == sVisited.end()) vNodes.emplace_back(item);
			}
			//std::cout << pNode->name() << std::endl;
			sVisited.emplace(pNode);
			vNodes.pop_front();
		}

		//����ͼ�ṹ������������
		std::vector<Node*> vSorted;
		while (g.size()) {
			for (auto itr=g.begin();itr!=g.end();++itr)
			{
				//û������ڵ�
				auto& f = g[itr->first];
				if (f.first.size() == 0) {
					vSorted.push_back(itr->first);
					//�ҵ�ͼ������ڵ������ڵ㣬Ȼ������ڵ��Ӧ��������ڵ��Ƴ�
					auto outputs = f.second;//f['out']
					for (auto& output: outputs)
					{
						g[output].first.erase(itr->first);
					}
					//Ȼ������ڵ��ͼ���Ƴ�
					g.erase(itr->first);
					break;
				}
			}
		}
		return vSorted;
	}

	DLL_EXPORT void train_one_batch(std::vector<Node*>& graph)
	{

	}

	DLL_EXPORT void sgd_update(Node* startNode)
	{

	}

}